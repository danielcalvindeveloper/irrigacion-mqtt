#include <Arduino.h>
#include <ArduinoOTA.h>
#include <ESP8266WebServer.h>
#include <ArduinoJson.h>
#include "config/Config.h"
#include "config/Secrets.h"
#include "network/WiFiManager.h"
#include "network/TimeSync.h"
#include "network/MqttManager.h"
#include "network/HttpClient.h"
#include "hardware/RelayController.h"
#include "storage/SPIFFSManager.h"
#include "scheduler/AgendaManager.h"
#include "display/DisplayManager.h"
#include "utils/Logger.h"

// ============================================================================
// FIRMWARE ESP8266 - SISTEMA DE RIEGO MQTT
// ============================================================================

// Forward declarations de funciones
void initHardware();
void initSerial();
void printBanner();
void mainLoop();
void onMqttCommand(int zona, String accion, int duracion);
void onAgendaSync(String payload);
void onZoneStateChanged(int zona, bool estado);
void onRiegoEvent(int zona, String evento, String origen, int duracion, int versionAgenda);
void showStoredAgenda();
void fetchAndStoreAgendas();
void initOTA();
bool loadWiFiConfig(String& ssid, String& password,
                    String& mqttHost, uint16_t& mqttPort,
                    String& mqttUser, String& mqttPassword,
                    String& backendHost, uint16_t& backendPort,
                    String& backendUser, String& backendPassword,
                    String& nodeId);
bool saveWiFiConfig(const String& ssid, const String& password,
                    const String& mqttHost, uint16_t mqttPort,
                    const String& mqttUser, const String& mqttPassword,
                    const String& backendHost, uint16_t backendPort,
                    const String& backendUser, const String& backendPassword,
                    const String& nodeId);
void runConfigPortal(bool factoryReset);
void handleFactoryResetButton();

// Estado global del sistema
SystemState currentState = INIT;
unsigned long lastStatusPublish = 0;
unsigned long lastHumidityRead = 0;
unsigned long lastDisplayUpdate = 0;
bool otaInitialized = false;
String activeWiFiSsid = WIFI_SSID;
String activeWiFiPassword = WIFI_PASSWORD;
String activeMqttHost = MQTT_BROKER;
uint16_t activeMqttPort = MQTT_PORT;
String activeMqttUser = MQTT_USER;
String activeMqttPassword = MQTT_PASSWORD;
String activeBackendHost = BACKEND_HOST;
uint16_t activeBackendPort = BACKEND_PORT;
String activeBackendUser = BACKEND_USER;
String activeBackendPassword = BACKEND_PASSWORD;
String activeNodeId = NODE_ID;
unsigned long factoryButtonPressStart = 0;

// Módulos del sistema
WiFiManager wifiManager;
TimeSync timeSync;
MqttManager mqttManager;
HttpClient httpClient;
RelayController relayController;
SPIFFSManager spiffsManager;
AgendaManager* agendaManager = nullptr;
DisplayManager displayManager;

// Portal de configuración (solo bajo acción física explícita)
ESP8266WebServer configServer(80);
// ============================================================================
// SETUP - Inicialización del sistema
// ============================================================================
void setup() {
    // Inicializar comunicación serial
    initSerial();
    
    // Banner de inicio
    printBanner();
    
    // Inicializar hardware
    initHardware();
    
    // Inicializar Display OLED
    displayManager.init(I2C_SDA, I2C_SCL, OLED_ADDRESS, OLED_WIDTH, OLED_HEIGHT);
    displayManager.clear();
    displayManager.showStatusLine("Configurando...");
    displayManager.display();
    
    // Inicializar módulos
    Logger::info("Inicializando módulos del sistema...");
    
    // SPIFFSManager (primero para poder leer configuracion)
    displayManager.showStatusLine("Preparando almacen.");
    displayManager.display();
    spiffsManager.init();
    spiffsManager.printInfo();

    // Cargar credenciales WiFi persistidas (si no existen, usar Secrets.h)
    if (loadWiFiConfig(activeWiFiSsid, activeWiFiPassword,
                       activeMqttHost, activeMqttPort,
                       activeMqttUser, activeMqttPassword,
                       activeBackendHost, activeBackendPort,
                       activeBackendUser, activeBackendPassword,
                       activeNodeId)) {
        Logger::logf(LOG_LEVEL_INFO, "Config runtime cargada desde %s", CONFIG_FILE);
    } else {
        Logger::info("Config runtime no encontrada, usando valores compilados");
    }
    
    // Mostrar agenda almacenada si existe
    showStoredAgenda();
    
    // WiFiManager
    displayManager.showStatusLine("Iniciando WiFi...");
    displayManager.display();
    wifiManager.setCredentials(activeWiFiSsid, activeWiFiPassword);
    wifiManager.init();
    
    // TimeSync (se sincronizará cuando WiFi esté conectado)
    timeSync.init();
    
    // HttpClient (para solicitar agendas al backend)
    httpClient.setRuntimeConfig(activeBackendHost, activeBackendPort, activeBackendUser, activeBackendPassword, activeNodeId);
    httpClient.init();
    
    // MqttManager
    displayManager.showStatusLine("Iniciando MQTT...");
    displayManager.display();
    mqttManager.setRuntimeConfig(activeMqttHost, activeMqttPort, activeMqttUser, activeMqttPassword, activeNodeId);
    mqttManager.init();
    mqttManager.setCommandCallback(onMqttCommand);
    mqttManager.setAgendaSyncCallback(onAgendaSync);
    
    // RelayController
    displayManager.showStatusLine("Iniciando reles...");
    displayManager.display();
    relayController.init();
    relayController.setStateChangedCallback(onZoneStateChanged);
    relayController.setRiegoEventCallback(onRiegoEvent);
    
    // AgendaManager (requiere SPIFFSManager, TimeSync, RelayController, MqttManager)
    displayManager.showStatusLine("Preparando agendas");
    displayManager.display();
    agendaManager = new AgendaManager(&spiffsManager, &timeSync, &relayController, &mqttManager);
    agendaManager->init();
    
    // TODO: Inicializar otros modulos
    // - HumiditySensor (hardware no disponible)
    
    Logger::info("Sistema inicializado - iniciando conexión WiFi");
    
    // Cambiar estado
    currentState = WIFI_CONNECTING;
}

// ============================================================================
// LOOP - Loop principal
// ============================================================================
void loop() {
    handleFactoryResetButton();

    if (OTA_ENABLED && wifiManager.isConnected() && !otaInitialized) {
        initOTA();
    }

    // Actualizar módulos
    wifiManager.loop();
    mqttManager.loop();
    if (OTA_ENABLED && otaInitialized) {
        ArduinoOTA.handle();
    }
    relayController.loop();  // CRITICO: actualizar timers de zonas
    
    // TimeSync solo actualiza si WiFi está conectado
    if (wifiManager.isConnected()) {
        timeSync.loop();
    }
    
    // AgendaManager - verificar y ejecutar agendas programadas
    if (agendaManager != nullptr) {
        agendaManager->loop();
    }
    
    // Actualizar iconos de estado en display cada 2 segundos
    unsigned long now = millis();
    if (now - lastDisplayUpdate >= 2000) {
        lastDisplayUpdate = now;
        int rssi = wifiManager.isConnected() ? wifiManager.getRSSI() : -100;
        displayManager.updateStatusIcons(rssi, wifiManager.isConnected(), mqttManager.isConnected());
        
        // Actualizar fecha/hora en centro superior
        time_t rawtime = timeSync.getEpoch();
        struct tm* timeinfo = localtime(&rawtime);
        if (timeinfo != nullptr) {
            displayManager.updateDateTimeDisplay(
                timeinfo->tm_wday,      // 0=Domingo, 1=Lunes, etc
                timeinfo->tm_mday,      // Día del mes (1-31)
                timeinfo->tm_hour,      // Hora (0-23)
                timeinfo->tm_min        // Minuto (0-59)
            );
        }
        
        // Actualizar indicadores de zonas
        bool zone1Active = relayController.isActive(1);
        bool zone2Active = relayController.isActive(2);
        bool zone3Active = relayController.isActive(3);
        bool zone4Active = relayController.isActive(4);
        displayManager.updateZoneIndicators(zone1Active, zone2Active, zone3Active, zone4Active);
        
        displayManager.display();
    }
    
    // Máquina de estados principal
    mainLoop();
    
    // Delay para evitar watchdog timeout
    delay(LOOP_DELAY_MS);
}

// ============================================================================
// FUNCIONES DE INICIALIZACIÓN
// ============================================================================

void initSerial() {
    Serial.begin(SERIAL_BAUD_RATE);
    delay(1000);  // Esperar a que serial esté listo
    
    while (!Serial && millis() < 5000) {
        // Esperar hasta 5 segundos para serial
        delay(100);
    }
}

void initHardware() {
    Serial.println("[INFO] Inicializando hardware...");

    pinMode(FACTORY_RESET_BUTTON_PIN, INPUT_PULLUP);
    
    // LED integrado - Deshabilitado: pin GPIO2 usado para I2C_SDA
    // pinMode(LED_PIN, OUTPUT);
    // digitalWrite(LED_PIN, LOW);
    
    // Pines de relés (todos OFF al inicio)
    for (int i = 0; i < MAX_ZONES; i++) {
        pinMode(RELAY_PINS[i], OUTPUT);
        digitalWrite(RELAY_PINS[i], RELAY_OFF);
    }
    Serial.printf("[INFO] %d relés inicializados (todos OFF)\n", MAX_ZONES);
    
    // Pines de sensores (INPUT)
    for (int i = 0; i < MAX_SENSORS; i++) {
        pinMode(SENSOR_PINS[i], INPUT);
    }
    Serial.printf("[INFO] %d sensores ADC configurados\n", MAX_SENSORS);
    Serial.printf("[INFO] Boton config/factory en GPIO%d (INPUT_PULLUP)\n", FACTORY_RESET_BUTTON_PIN);
    
    // Parpadeo de LED deshabilitado - LED_PIN no disponible
    // for (int i = 0; i < 3; i++) {
    //     digitalWrite(LED_PIN, HIGH);
    //     delay(200);
    //     digitalWrite(LED_PIN, LOW);
    //     delay(200);
    // }
}

void printBanner() {
    Serial.println("\n\n");
    Serial.println("====================================");
    Serial.println("  SISTEMA DE RIEGO ESP32 - MQTT");
    Serial.println("====================================");
    Serial.printf(" Firmware: v%s\n", FIRMWARE_VERSION);
    Serial.printf(" Build: %s %s\n", BUILD_DATE, BUILD_TIME);
    Serial.printf(" Node ID: %s\n", activeNodeId.c_str());
    Serial.printf(" Max Zonas: %d\n", MAX_ZONES);
    Serial.printf(" Sensores: %d\n", MAX_SENSORS);
    Serial.printf(" OTA: %s (puerto %d)\n", OTA_ENABLED ? "habilitado" : "deshabilitado", OTA_PORT);
    Serial.println("====================================");
    Serial.println();
}

void initOTA() {
    String otaHost = String("riego-") + activeNodeId;
    otaHost.replace(" ", "-");

    ArduinoOTA.setPort(OTA_PORT);
    ArduinoOTA.setHostname(otaHost.c_str());

    ArduinoOTA.onStart([]() {
        Logger::info("OTA iniciado");
        displayManager.showStatusLine("OTA: iniciando...");
        displayManager.display();
    });

    ArduinoOTA.onEnd([]() {
        Logger::info("OTA completado");
        displayManager.showStatusLine("OTA: completo");
        displayManager.display();
    });

    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
        const unsigned int percentage = (total > 0) ? ((progress * 100U) / total) : 0;
        static unsigned int lastPercentage = 0;
        if (percentage >= lastPercentage + 10 || percentage == 100) {
            lastPercentage = percentage;
            Logger::info("OTA progreso: " + String(percentage) + "%");
        }
    });

    ArduinoOTA.onError([](ota_error_t error) {
        Logger::error("OTA error: " + String((int)error));
        displayManager.showStatusLine("OTA: error");
        displayManager.display();
    });

    ArduinoOTA.begin();
    otaInitialized = true;

    Logger::info("OTA listo. Host: " + otaHost + " Puerto: " + String(OTA_PORT));
}

bool loadWiFiConfig(String& ssid, String& password,
                    String& mqttHost, uint16_t& mqttPort,
                    String& mqttUser, String& mqttPassword,
                    String& backendHost, uint16_t& backendPort,
                    String& backendUser, String& backendPassword,
                    String& nodeId) {
    if (!spiffsManager.isInitialized() || !spiffsManager.exists(CONFIG_FILE)) {
        return false;
    }

    String raw = spiffsManager.readFile(CONFIG_FILE);
    if (raw.length() == 0) {
        return false;
    }

    StaticJsonDocument<JSON_BUFFER_SMALL> doc;
    DeserializationError err = deserializeJson(doc, raw);
    if (err) {
        Logger::logf(LOG_LEVEL_ERROR, "Config WiFi invalida en %s", CONFIG_FILE);
        return false;
    }

    const char* cfgSsid = doc["wifiSsid"] | "";
    const char* cfgPassword = doc["wifiPassword"] | "";
    if (strlen(cfgSsid) == 0) {
        return false;
    }

    ssid = String(cfgSsid);
    password = String(cfgPassword);

    mqttHost = String(doc["mqttHost"] | MQTT_BROKER);
    mqttPort = (uint16_t)(doc["mqttPort"] | MQTT_PORT);
    mqttUser = String(doc["mqttUser"] | MQTT_USER);
    mqttPassword = String(doc["mqttPassword"] | MQTT_PASSWORD);

    backendHost = String(doc["backendHost"] | BACKEND_HOST);
    backendPort = (uint16_t)(doc["backendPort"] | BACKEND_PORT);
    backendUser = String(doc["backendUser"] | BACKEND_USER);
    backendPassword = String(doc["backendPassword"] | BACKEND_PASSWORD);

    nodeId = String(doc["nodeId"] | NODE_ID);
    if (nodeId.length() == 0) {
        nodeId = NODE_ID;
    }

    return true;
}

bool saveWiFiConfig(const String& ssid, const String& password,
                    const String& mqttHost, uint16_t mqttPort,
                    const String& mqttUser, const String& mqttPassword,
                    const String& backendHost, uint16_t backendPort,
                    const String& backendUser, const String& backendPassword,
                    const String& nodeId) {
    if (!spiffsManager.isInitialized() || ssid.length() == 0) {
        return false;
    }

    StaticJsonDocument<JSON_BUFFER_LARGE> doc;
    doc["wifiSsid"] = ssid;
    doc["wifiPassword"] = password;
    doc["mqttHost"] = mqttHost;
    doc["mqttPort"] = mqttPort;
    doc["mqttUser"] = mqttUser;
    doc["mqttPassword"] = mqttPassword;
    doc["backendHost"] = backendHost;
    doc["backendPort"] = backendPort;
    doc["backendUser"] = backendUser;
    doc["backendPassword"] = backendPassword;
    doc["nodeId"] = nodeId;

    String out;
    serializeJson(doc, out);
    return spiffsManager.writeFile(CONFIG_FILE, out);
}

void runConfigPortal(bool factoryReset) {
    Logger::warn("Entrando a portal de configuracion WiFi por boton fisico");

    if (factoryReset) {
        spiffsManager.deleteFile(CONFIG_FILE);
        Logger::warn("Factory reset: configuracion persistida eliminada");
    }

    mqttManager.disconnect();
    wifiManager.disconnect();

    displayManager.showStatusLine("Portal config activo");
    displayManager.display();

    WiFi.mode(WIFI_AP);

    String apSsid = String("RIEGO-CONFIG-") + String(ESP.getChipId(), HEX);
    bool apOk = WiFi.softAP(apSsid.c_str(), CONFIG_PORTAL_AP_PASSWORD);
    if (!apOk) {
        Logger::error("No se pudo iniciar AP de configuracion");
        return;
    }

    Logger::logf(LOG_LEVEL_INFO, "AP config: SSID=%s IP=%s", apSsid.c_str(), WiFi.softAPIP().toString().c_str());

    bool shouldReboot = false;
    bool testInProgress = false;
    bool testDone = false;
    bool testSuccess = false;
    unsigned long testStartTs = 0;
    String testSsid = "";
    String testPassword = "";
    String testMqttHost = "";
    uint16_t testMqttPort = 0;
    String testMqttUser = "";
    String testMqttPassword = "";
    String testBackendHost = "";
    uint16_t testBackendPort = 0;
    String testBackendUser = "";
    String testBackendPassword = "";
    String testNodeId = "";
    String testMessage = "";

    auto buildPage = [&](const String& statusMessage, bool isError) {
        int networkCount = WiFi.scanNetworks();
        String networkList = "";

        if (networkCount <= 0) {
            networkList = "<p class='hint'>No se encontraron redes en este escaneo.</p>";
        } else {
            networkList = "<ul>";
            for (int i = 0; i < networkCount; i++) {
                String ssidName = WiFi.SSID(i);
                String safeAttr = ssidName;
                safeAttr.replace("&", "&amp;");
                safeAttr.replace("<", "&lt;");
                safeAttr.replace(">", "&gt;");
                safeAttr.replace("\"", "&quot;");

                String jsSsid = ssidName;
                jsSsid.replace("\\", "\\\\");
                jsSsid.replace("'", "\\'");

                int signal = WiFi.RSSI(i);
                String secure = (WiFi.encryptionType(i) == ENC_TYPE_NONE) ? "Abierta" : "Protegida";

                networkList += "<li><button type='button' class='net' onclick=\"pick('" + jsSsid + "')\">" +
                               safeAttr + " <span>(" + signal + " dBm, " + secure + ")</span></button></li>";
            }
            networkList += "</ul>";
        }

        String statusBlock = "";
        if (statusMessage.length() > 0) {
            String statusClass = isError ? "err" : "ok";
            statusBlock = "<div class='status " + statusClass + "'>" + statusMessage + "</div>";
        }

        String page =
            "<!doctype html><html><head><meta charset='utf-8'>"
            "<meta name='viewport' content='width=device-width,initial-scale=1'>"
            "<title>Config WiFi</title>"
            "<style>body{font-family:Arial,sans-serif;background:#f4f7f9;padding:16px;margin:0;}"
            ".card{max-width:560px;margin:0 auto;background:#fff;padding:18px;border-radius:10px;}"
            "h2{margin-top:0;}input{width:100%;padding:10px;margin:8px 0;box-sizing:border-box;}"
            "button{padding:10px 12px;border:none;border-radius:8px;cursor:pointer;}"
            ".primary{width:100%;background:#1f7a8c;color:#fff;font-weight:600;}"
            ".muted{background:#e8eef2;color:#223;display:inline-block;margin-bottom:8px;}"
            ".hint{color:#556;font-size:13px;}ul{list-style:none;padding:0;margin:8px 0 0 0;}"
            "li{margin:6px 0;}.net{width:100%;text-align:left;background:#f8fafb;border:1px solid #dbe5ea;}"
            ".net span{color:#566;}.status{padding:10px;border-radius:8px;margin-bottom:10px;}"
            ".status.ok{background:#e8f6e8;color:#114e1a;border:1px solid #b8e1bc;}"
            ".status.err{background:#fdeaea;color:#7d1a1a;border:1px solid #f3b7b7;}</style>"
            "<script>function pick(v){document.getElementById('ssid').value=v;}</script>"
            "</head><body><div class='card'><h2>Configurar WiFi</h2>" +
            statusBlock +
            "<p class='hint'>AP actual: <b>" + apSsid + "</b> | IP portal: <b>" + WiFi.softAPIP().toString() + "</b></p>"
            "<a href='/'><button type='button' class='muted'>Reescanear redes</button></a>"
            "<form method='POST' action='/save'>"
            "<label>SSID</label><input id='ssid' name='ssid' maxlength='32' required value='" + activeWiFiSsid + "'>"
            "<label>Password</label><input name='password' type='password' maxlength='64' value='" + activeWiFiPassword + "'>"
            "<h3>Configuracion avanzada</h3>"
            "<label>MQTT Host</label><input name='mqttHost' maxlength='64' required value='" + activeMqttHost + "'>"
            "<label>MQTT Port</label><input name='mqttPort' type='number' min='1' max='65535' required value='" + String(activeMqttPort) + "'>"
            "<label>MQTT User</label><input name='mqttUser' maxlength='64' value='" + activeMqttUser + "'>"
            "<label>MQTT Password</label><input name='mqttPassword' type='password' maxlength='64' value='" + activeMqttPassword + "'>"
            "<label>Backend Host</label><input name='backendHost' maxlength='64' required value='" + activeBackendHost + "'>"
            "<label>Backend Port</label><input name='backendPort' type='number' min='1' max='65535' required value='" + String(activeBackendPort) + "'>"
            "<label>Backend User</label><input name='backendUser' maxlength='64' value='" + activeBackendUser + "'>"
            "<label>Backend Password</label><input name='backendPassword' type='password' maxlength='64' value='" + activeBackendPassword + "'>"
            "<label>Node ID</label><input name='nodeId' maxlength='64' required value='" + activeNodeId + "'>"
            "<button class='primary' type='submit'>Probar conexion, guardar y reiniciar</button></form>"
            "<h3>Redes detectadas</h3>" + networkList +
            "<p class='hint'>Si no aparece tu red, acercate al router y tocá Reescanear.</p>"
            "</div></body></html>";

        return page;
    };

    configServer.on("/", HTTP_GET, [&]() {
        configServer.send(200, "text/html", buildPage("", false));
    });

    configServer.on("/save", HTTP_POST, [&]() {
        String ssid = configServer.arg("ssid");
        String password = configServer.arg("password");
        String mqttHost = configServer.arg("mqttHost");
        String mqttPortRaw = configServer.arg("mqttPort");
        String mqttUser = configServer.arg("mqttUser");
        String mqttPassword = configServer.arg("mqttPassword");
        String backendHost = configServer.arg("backendHost");
        String backendPortRaw = configServer.arg("backendPort");
        String backendUser = configServer.arg("backendUser");
        String backendPassword = configServer.arg("backendPassword");
        String nodeId = configServer.arg("nodeId");

        ssid.trim();
        mqttHost.trim();
        backendHost.trim();
        nodeId.trim();

        int mqttPortParsed = mqttPortRaw.toInt();
        int backendPortParsed = backendPortRaw.toInt();

        if (ssid.length() == 0) {
            configServer.send(400, "text/html", buildPage("SSID obligatorio", true));
            return;
        }

        if (mqttHost.length() == 0 || mqttPortParsed <= 0 || mqttPortParsed > 65535) {
            configServer.send(400, "text/html", buildPage("MQTT host/puerto inválido", true));
            return;
        }

        if (backendHost.length() == 0 || backendPortParsed <= 0 || backendPortParsed > 65535) {
            configServer.send(400, "text/html", buildPage("Backend host/puerto inválido", true));
            return;
        }

        if (nodeId.length() == 0) {
            configServer.send(400, "text/html", buildPage("Node ID obligatorio", true));
            return;
        }

        // Lanzar prueba no bloqueante para que el navegador siempre reciba feedback.
        testSsid = ssid;
        testPassword = password;
        testMqttHost = mqttHost;
        testMqttPort = (uint16_t)mqttPortParsed;
        testMqttUser = mqttUser;
        testMqttPassword = mqttPassword;
        testBackendHost = backendHost;
        testBackendPort = (uint16_t)backendPortParsed;
        testBackendUser = backendUser;
        testBackendPassword = backendPassword;
        testNodeId = nodeId;
        testMessage = "Probando conexion...";
        testDone = false;
        testSuccess = false;
        testInProgress = true;
        testStartTs = millis();

        displayManager.showStatusLine("Probando WiFi...");
        displayManager.display();

        WiFi.mode(WIFI_AP_STA);
        WiFi.begin(testSsid.c_str(), testPassword.c_str());

        String statusPage =
            "<!doctype html><html><head><meta charset='utf-8'>"
            "<meta name='viewport' content='width=device-width,initial-scale=1'>"
            "<meta http-equiv='refresh' content='2;url=/result'>"
            "<title>Probando WiFi</title>"
            "<style>body{font-family:Arial,sans-serif;background:#f4f7f9;padding:16px;}"
            ".card{max-width:520px;margin:0 auto;background:#fff;padding:18px;border-radius:10px;}"
            "</style></head><body><div class='card'><h2>Probando conexion</h2>"
            "<p>SSID: <b>" + testSsid + "</b></p>"
            "<p>Esta pagina se actualiza sola en unos segundos...</p>"
            "</div></body></html>";
        configServer.send(200, "text/html", statusPage);
    });

    configServer.on("/result", HTTP_GET, [&]() {
        if (testInProgress) {
            String waitPage =
                "<!doctype html><html><head><meta charset='utf-8'>"
                "<meta name='viewport' content='width=device-width,initial-scale=1'>"
                "<meta http-equiv='refresh' content='2;url=/result'>"
                "<title>Probando WiFi</title>"
                "<style>body{font-family:Arial,sans-serif;background:#f4f7f9;padding:16px;}"
                ".card{max-width:520px;margin:0 auto;background:#fff;padding:18px;border-radius:10px;}"
                "</style></head><body><div class='card'><h2>Probando conexion</h2>"
                "<p>SSID: <b>" + testSsid + "</b></p>"
                "<p>Aun en progreso, por favor espera...</p>"
                "</div></body></html>";
            configServer.send(200, "text/html", waitPage);
            return;
        }

        if (testDone && testSuccess) {
            String okPage =
                "<!doctype html><html><head><meta charset='utf-8'>"
                "<meta name='viewport' content='width=device-width,initial-scale=1'>"
                "<title>WiFi OK</title>"
                "<style>body{font-family:Arial,sans-serif;background:#f4f7f9;padding:16px;}"
                ".card{max-width:520px;margin:0 auto;background:#fff;padding:18px;border-radius:10px;}"
                ".ok{background:#e8f6e8;color:#114e1a;border:1px solid #b8e1bc;padding:10px;border-radius:8px;}"
                ".btn{display:block;width:100%;padding:12px;margin-top:12px;background:#1f7a8c;color:#fff;text-decoration:none;text-align:center;border-radius:8px;}"
                "</style></head><body><div class='card'><h2>Conexion exitosa</h2>"
                "<div class='ok'>" + testMessage + "</div>"
                "<p>La configuracion ya esta guardada en flash.</p>"
                "<a class='btn' href='/reboot'>Reiniciar ahora</a>"
                "</div></body></html>";
            configServer.send(200, "text/html", okPage);
            return;
        }

        configServer.send(200, "text/html", buildPage(testMessage, true));
    });

    configServer.on("/reboot", HTTP_GET, [&]() {
        String rebootPage =
            "<!doctype html><html><head><meta charset='utf-8'>"
            "<meta name='viewport' content='width=device-width,initial-scale=1'>"
            "<title>Reiniciando</title>"
            "<style>body{font-family:Arial,sans-serif;background:#f4f7f9;padding:16px;}"
            ".card{max-width:520px;margin:0 auto;background:#fff;padding:18px;border-radius:10px;}"
            "</style></head><body><div class='card'><h2>Reiniciando equipo</h2>"
            "<p>Puedes cerrar esta pagina y reconectar cuando el equipo vuelva a la red.</p>"
            "</div></body></html>";
        configServer.send(200, "text/html", rebootPage);
        delay(250);
        shouldReboot = true;
    });

    configServer.begin();

    while (!shouldReboot) {
        configServer.handleClient();

        if (testInProgress) {
            if (WiFi.status() == WL_CONNECTED) {
                if (!saveWiFiConfig(testSsid, testPassword,
                                    testMqttHost, testMqttPort,
                                    testMqttUser, testMqttPassword,
                                    testBackendHost, testBackendPort,
                                    testBackendUser, testBackendPassword,
                                    testNodeId)) {
                    testMessage = "Conecto OK, pero fallo al guardar en flash.";
                    testSuccess = false;
                } else {
                    activeWiFiSsid = testSsid;
                    activeWiFiPassword = testPassword;
                    activeMqttHost = testMqttHost;
                    activeMqttPort = testMqttPort;
                    activeMqttUser = testMqttUser;
                    activeMqttPassword = testMqttPassword;
                    activeBackendHost = testBackendHost;
                    activeBackendPort = testBackendPort;
                    activeBackendUser = testBackendUser;
                    activeBackendPassword = testBackendPassword;
                    activeNodeId = testNodeId;
                    testMessage = String("Conectado correctamente a ") + testSsid +
                                 " (IP: " + WiFi.localIP().toString() + ")";
                    testSuccess = true;
                    displayManager.showStatusLine("WiFi OK");
                    displayManager.display();
                }
                testDone = true;
                testInProgress = false;
            } else if (millis() - testStartTs >= WIFI_TIMEOUT) {
                WiFi.disconnect();
                WiFi.mode(WIFI_AP);
                WiFi.softAP(apSsid.c_str(), CONFIG_PORTAL_AP_PASSWORD);
                testMessage = "No se pudo conectar. Verifica SSID/clave e intenta otra vez.";
                testSuccess = false;
                testDone = true;
                testInProgress = false;
                displayManager.showStatusLine("WiFi fallo");
                displayManager.display();
            }
        }

        delay(10);
    }

    configServer.stop();
    delay(200);
    ESP.restart();
}

void handleFactoryResetButton() {
    int buttonState = digitalRead(FACTORY_RESET_BUTTON_PIN);

    if (buttonState == LOW) {
        if (factoryButtonPressStart == 0) {
            factoryButtonPressStart = millis();
        }
        return;
    }

    if (factoryButtonPressStart == 0) {
        return;
    }

    unsigned long pressedMs = millis() - factoryButtonPressStart;
    factoryButtonPressStart = 0;

    if (pressedMs >= FACTORY_RESET_HOLD_MS) {
        Logger::warn("Boton: FACTORY RESET (hold largo)");
        runConfigPortal(true);
    } else if (pressedMs >= CONFIG_PORTAL_HOLD_MS) {
        Logger::warn("Boton: abrir portal de configuracion");
        runConfigPortal(false);
    }
}

// ============================================================================
// MÁQUINA DE ESTADOS
// ============================================================================

void mainLoop() {
    static unsigned long lastStateChange = 0;
    static bool statusMessageShown = false;
    
    // Log de cambio de estado
    static SystemState lastState = INIT;
    if (currentState != lastState) {
        Serial.printf("[INFO] Estado cambiado: %s → %s\n", 
                      STATE_NAMES[lastState], 
                      STATE_NAMES[currentState]);
        lastState = currentState;
        lastStateChange = millis();
        statusMessageShown = false;
    }
    
    // Procesar según estado actual
    switch (currentState) {
        case INIT:
            // Estado inicial, ya manejado en setup()
            break;
            
        case WIFI_CONNECTING:
            if (!statusMessageShown) {
                displayManager.showStatusLine("Conectando WiFi...");
                displayManager.display();
                statusMessageShown = true;
            }
            
            // Intentar conectar WiFi
            if (wifiManager.isConnected()) {
                Logger::info("WiFi conectado exitosamente");
                wifiManager.printInfo();
                currentState = WIFI_CONNECTED;
            } else if (millis() - lastStateChange > WIFI_TIMEOUT * 2) {
                // Timeout extendido: Si después de varios intentos no se conecta
                Logger::warn("WiFi no disponible - entrando en modo OFFLINE");
                displayManager.showStatusLine("WiFi no disponible");
                displayManager.display();
                currentState = OFFLINE;
            } else if (millis() - lastStateChange > 5000) {
                // Primer intento de conexión después de 5 segundos
                static bool firstAttempt = true;
                if (firstAttempt) {
                    Logger::info("Iniciando conexión WiFi...");
                    wifiManager.connect();
                    firstAttempt = false;
                }
            }
            break;
            
        case WIFI_CONNECTED:
            if (!statusMessageShown) {
                displayManager.showStatusLine("Sinc. fecha y hora");
                displayManager.display();
                statusMessageShown = true;
            }
            
            // Sincronizar tiempo con NTP
            if (!timeSync.isSynchronized()) {
                Logger::info("Sincronizando tiempo con NTP...");
                if (timeSync.sync()) {
                    Logger::info("Tiempo sincronizado correctamente");
                    currentState = MQTT_CONNECTING;
                } else {
                    // Si falla NTP, continuar igual (no es crítico)
                    Logger::warn("Fallo sincronización NTP, continuando sin tiempo exacto");
                    currentState = MQTT_CONNECTING;
                }
            } else {
                currentState = MQTT_CONNECTING;
            }
            break;
            
        case MQTT_CONNECTING:
            if (!statusMessageShown) {
                displayManager.showStatusLine("Conectando MQTT...");
                displayManager.display();
                statusMessageShown = true;
            }
            
            // Intentar conectar MQTT
            if (mqttManager.isConnected()) {
                Logger::info("MQTT conectado, sistema ONLINE");
                mqttManager.printInfo();
                currentState = ONLINE;
            } else if (millis() - lastStateChange > MQTT_RECONNECT_DELAY * 2) {
                // Si MQTT falla despues de varios intentos, volver a verificar WiFi
                Logger::warn("MQTT no disponible, verificando WiFi...");
                if (wifiManager.isConnected()) {
                    // WiFi OK pero MQTT falla, reintentar
                    static unsigned long lastMqttRetry = 0;
                    if (millis() - lastMqttRetry > 30000) {
                        Logger::info("Reintentando conexion MQTT...");
                        mqttManager.connect();
                        lastMqttRetry = millis();
                    }
                } else {
                    // WiFi caido, volver a conectar
                    currentState = WIFI_CONNECTING;
                }
            } else if (millis() - lastStateChange > 5000) {
                // Primer intento despues de 5 segundos
                static bool firstMqttAttempt = true;
                if (firstMqttAttempt) {
                    Logger::info("Iniciando conexion MQTT...");
                    mqttManager.connect();
                    firstMqttAttempt = false;
                }
            }
            break;
            
        case ONLINE:
            // Actualizar mensaje de estado al entrar
            if (!statusMessageShown) {
                displayManager.showStatusLine("Actualizando agenda");
                displayManager.display();
                statusMessageShown = true;
            }
            
            // Modo online completo - verificar que todo sigue conectado
            if (!wifiManager.isConnected()) {
                Logger::warn("WiFi desconectado, volviendo a WIFI_CONNECTING");
                currentState = WIFI_CONNECTING;
            } else if (!mqttManager.isConnected()) {
                Logger::warn("MQTT desconectado, volviendo a MQTT_CONNECTING");
                currentState = MQTT_CONNECTING;
            }
            
            // Solicitar agendas del backend al entrar en modo ONLINE (una vez)
            {
                static bool agendasFetched = false;
                static unsigned long lastFetchAttempt = 0;
                
                if (!agendasFetched) {
                    Logger::info("Solicitando agendas al backend...");
                    fetchAndStoreAgendas();
                    agendasFetched = true;
                    lastFetchAttempt = millis();
                    
                    // Cambiar mensaje después de actualizar agendas
                    displayManager.showStatusLine("Sistema listo");
                    displayManager.display();
                } else {
                    // Reintentar cada 5 minutos si la primera vez falló
                    if (millis() - lastFetchAttempt > 300000) { // 5 minutos
                        Logger::info("Reintentando sincronización de agendas...");
                        fetchAndStoreAgendas();
                        lastFetchAttempt = millis();
                    }
                }
            }
            
            // Publicar estado de zonas activas cada 5 segundos
            {
                static unsigned long lastStatusPublish = 0;
                if (millis() - lastStatusPublish > 5000) {
                    for (int zona = 1; zona <= MAX_ZONES; zona++) {
                        if (relayController.isActive(zona)) {
                            int remaining = relayController.getRemainingTime(zona);
                            mqttManager.publishZoneStatus(zona, true, remaining);
                        }
                    }
                    lastStatusPublish = millis();
                }
            }
            
            break;
            
        case OFFLINE:
            if (!statusMessageShown) {
                displayManager.showStatusLine("Modo offline");
                displayManager.display();
                statusMessageShown = true;
            }
            
            // Modo offline - el WiFiManager intentará reconectar automáticamente
            // Si se reconecta, cambiar de estado
            if (wifiManager.isConnected()) {
                Logger::info("WiFi reconectado desde modo OFFLINE");
                currentState = WIFI_CONNECTED;
            }
            
            // TODO: Ejecutar agendas locales desde SPIFFS
            
            static unsigned long lastOfflineLog = 0;
            if (millis() - lastOfflineLog > 30000) {
                Logger::info("Modo OFFLINE - sistema funcionando de forma autonoma");
                lastOfflineLog = millis();
            }
            break;
    }
    
    // Tareas comunes independientes del estado
    // TODO: Implementar
    // - Actualizar timers de relés
    // - Verificar agendas programadas
    // - Leer sensores periódicamente
    
    // Indicador visual de vida - Deshabilitado: LED_PIN no disponible
    // static unsigned long lastBlink = 0;
    // if (millis() - lastBlink > 1000) {
    //     digitalWrite(LED_PIN, !digitalRead(LED_PIN));
    //     lastBlink = millis();
    // }
    
    // Debug: Mostrar tiempo cada 30 segundos si está sincronizado
    static unsigned long lastTimeDebug = 0;
    if (timeSync.isSynchronized() && millis() - lastTimeDebug > 30000) {
        Logger::logf(LOG_LEVEL_INFO, "Tiempo actual: %s", timeSync.getDateTimeString().c_str());
        lastTimeDebug = millis();
    }
}

// ============================================================================
// FUNCIONES AUXILIARES (STUBS)
// ============================================================================

// TODO: Implementar en módulos correspondientes
void publishZoneStatus() {
    // Publicar estado de todas las zonas via MQTT
}


// ============================================================================
// CALLBACKS MQTT
// ============================================================================

void onMqttCommand(int zona, String accion, int duracion) {
    Logger::logf(LOG_LEVEL_INFO, ">>> Comando recibido - Zona: %d, Accion: %s, Duracion: %d seg", 
               zona, accion.c_str(), duracion);
    
    // Ejecutar comando en relayController
    if (accion == "ON") {
        relayController.turnOn(zona, duracion);
    } else if (accion == "OFF") {
        relayController.turnOff(zona);
    }
    
    // Publicar estado actualizado inmediatamente con tiempo restante
    bool estado = relayController.isActive(zona);
    int remaining = relayController.getRemainingTime(zona);
    mqttManager.publishZoneStatus(zona, estado, remaining);
    
    // Log de confirmacion
    if (estado) {
        Logger::logf(LOG_LEVEL_INFO, "Zona %d encendida, tiempo restante: %d seg", zona, remaining);
    }
}

void onAgendaSync(String payload) {
    Logger::logf(LOG_LEVEL_INFO, ">>> Sincronizacion de agenda recibida (%d bytes)", payload.length());
    
    // Guardar agenda en SPIFFS
    if (spiffsManager.isInitialized()) {
        const char* agendaFile = "/agenda.json";
        
        if (spiffsManager.writeFile(agendaFile, payload)) {
            Logger::logf(LOG_LEVEL_INFO, "Agenda guardada en SPIFFS: %s", agendaFile);
            
            // Mostrar info de almacenamiento
            Logger::logf(LOG_LEVEL_INFO, "Espacio usado: %.1f%% (%d/%d bytes)", 
                       spiffsManager.getUsagePercent(),
                       spiffsManager.getUsedBytes(),
                       spiffsManager.getTotalBytes());
            
            // Listar archivos
            spiffsManager.listFiles();
            
            // Mostrar contenido de la agenda guardada
            showStoredAgenda();
            
            // Publicar evento de sincronización exitosa
            if (mqttManager.isConnected()) {
                String detalles = String("Agenda sincronizada correctamente (") + payload.length() + " bytes)";
                mqttManager.publishSystemEvent("agenda_sync_ok", detalles, -1);
            }
        } else {
            Logger::error("Error al guardar agenda en SPIFFS");
            
            // Publicar evento de error
            if (mqttManager.isConnected()) {
                String detalles = String("Error al guardar agenda en SPIFFS (") + payload.length() + " bytes, " + 
                                spiffsManager.getFreeBytes() + " bytes libres)";
                mqttManager.publishSystemEvent("agenda_storage_error", detalles, 0);
            }
        }
    } else {
        Logger::warn("SPIFFS no inicializado, agenda no guardada");
        
        // Publicar evento de error
        if (mqttManager.isConnected()) {
            mqttManager.publishSystemEvent("agenda_storage_error", "SPIFFS no inicializado", 0);
        }
    }
    
    // TODO: Parsear agenda y cargar en AgendaManager
}

void onZoneStateChanged(int zona, bool estado) {
    // Callback llamado cuando una zona cambia de estado (ej: auto-apagado por timer)
    Logger::logf(LOG_LEVEL_INFO, ">>> Cambio de estado zona %d: %s", zona, estado ? "ON" : "OFF");
    
    // Publicar estado actualizado via MQTT
    if (mqttManager.isConnected()) {
        int remaining = estado ? relayController.getRemainingTime(zona) : 0;
        mqttManager.publishZoneStatus(zona, estado, remaining);
    }
}

void onRiegoEvent(int zona, String evento, String origen, int duracion, int versionAgenda) {
    // Callback para eventos de inicio/fin de riego
    Logger::logf(LOG_LEVEL_INFO, ">>> Evento riego zona %d: %s (%s, %d seg)", 
                 zona, evento.c_str(), origen.c_str(), duracion);
    
    // Publicar evento via MQTT
    if (mqttManager.isConnected()) {
        mqttManager.publishRiegoEvento(zona, evento, origen, duracion, versionAgenda);
    } else {
        Logger::warn("MQTT no conectado - evento no publicado");
    }
}

void readAndPublishHumidity() {
    // Leer sensores y publicar via MQTT
}

void checkAndExecuteAgendas() {
    // Verificar si hay agendas para ejecutar
}

void updateRelayTimers() {
    // Decrementar timers y apagar relés cuando lleguen a 0
}

// ============================================================================
// Mostrar agenda almacenada en SPIFFS
// ============================================================================
void showStoredAgenda() {
    const char* agendaFile = "/agenda.json";
    
    if (!spiffsManager.isInitialized()) {
        Logger::warn("SPIFFS no inicializado, no se puede leer agenda");
        return;
    }
    
    if (!spiffsManager.exists(agendaFile)) {
        Logger::info("No hay agenda almacenada en SPIFFS");
        return;
    }
    
    String agendaContent = spiffsManager.readFile(agendaFile);
    
    if (agendaContent.length() == 0) {
        Logger::error("Archivo de agenda vacio o error al leer");
        return;
    }
    
    Serial.println("\n╔════════════════════════════════════════════════════════════════");
    Serial.println("║ AGENDA ALMACENADA EN SPIFFS");
    Serial.println("╠════════════════════════════════════════════════════════════════");
    Serial.printf("║ Archivo: %s\n", agendaFile);
    Serial.printf("║ Tamaño: %d bytes\n", agendaContent.length());
    Serial.println("╠════════════════════════════════════════════════════════════════");
    Serial.println("║ Contenido JSON:");
    Serial.println("╠────────────────────────────────────────────────────────────────");
    
    // Imprimir el JSON con indentación básica
    Serial.println(agendaContent);
    
    Serial.println("╚════════════════════════════════════════════════════════════════\n");
}

// ============================================================================
// Solicitar y almacenar agendas desde backend via HTTP
// ============================================================================
void fetchAndStoreAgendas() {
    if (!wifiManager.isConnected()) {
        Logger::warn("WiFi no conectado, no se pueden solicitar agendas");
        Logger::info("Continuando con agendas almacenadas localmente");
        
        // Publicar evento de advertencia
        if (mqttManager.isConnected()) {
            mqttManager.publishSystemEvent("agenda_fetch_warning", "WiFi no conectado - usando agendas locales", -1);
        }
        return;
    }
    
    // Obtener agendas desde backend
    String agendasJson = httpClient.fetchAgendas();
    
    if (agendasJson.length() == 0) {
        Logger::warn("No se pudieron obtener agendas del backend");
        
        // Verificar si hay agendas almacenadas localmente
        if (spiffsManager.exists("/agenda.json")) {
            Logger::info("Continuando con agendas almacenadas localmente");
            showStoredAgenda();
            
            // Publicar evento de advertencia
            if (mqttManager.isConnected()) {
                mqttManager.publishSystemEvent("agenda_fetch_warning", "Backend no disponible - usando agendas locales", -1);
            }
        } else {
            Logger::warn("No hay agendas locales - sistema funcionara sin agendas programadas");
            
            // Publicar evento de error crítico
            if (mqttManager.isConnected()) {
                mqttManager.publishSystemEvent("agenda_load_error", "Sin agendas: backend no disponible y sin cache local", 0);
            }
        }
        return;
    }
    
    Logger::info("Agendas obtenidas exitosamente del backend");
    
    // El HTTP retorna un array directo [...], pero el ESP8266 espera formato MQTT: {"agendas": [...]}
    // Envolver el array en un objeto para mantener compatibilidad con AgendaManager
    String wrappedJson = "{\"agendas\":" + agendasJson + "}";
    
    // Publicar evento de carga inicial exitosa
    if (mqttManager.isConnected()) {
        String detalles = String("Agendas cargadas desde backend HTTP (") + agendasJson.length() + " bytes)";
        mqttManager.publishSystemEvent("agenda_initial_load_ok", detalles, -1);
    }
    
    // Procesar como si fuera una sincronización MQTT
    onAgendaSync(wrappedJson);
}
