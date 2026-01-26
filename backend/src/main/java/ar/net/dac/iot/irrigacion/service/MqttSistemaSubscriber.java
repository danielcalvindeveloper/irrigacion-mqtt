package ar.net.dac.iot.irrigacion.service;

import com.fasterxml.jackson.databind.JsonNode;
import com.fasterxml.jackson.databind.ObjectMapper;
import com.hivemq.client.mqtt.mqtt5.Mqtt5BlockingClient;
import jakarta.annotation.PostConstruct;
import jakarta.annotation.PreDestroy;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.boot.autoconfigure.condition.ConditionalOnBean;
import org.springframework.stereotype.Service;

import java.nio.charset.StandardCharsets;
import java.util.Optional;
import java.util.UUID;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

/**
 * Subscriber para eventos de sistema publicados por el ESP8266/ESP32.
 * Topic: riego/{nodeId}/sistema/evento
 * Campos esperados: tipo, timestamp, detalles, memoriaLibre (opcional agendasCargadas).
 * Loguea según severidad y descarta silenciosamente mensajes malformados para no romper el backend.
 */
@Service
@ConditionalOnBean(Mqtt5BlockingClient.class)
public class MqttSistemaSubscriber {

    private static final Logger log = LoggerFactory.getLogger(MqttSistemaSubscriber.class);

    private final Mqtt5BlockingClient mqttClient;
    private final ObjectMapper objectMapper = new ObjectMapper();
    private final ExecutorService executor = Executors.newSingleThreadExecutor();
    private volatile boolean running = false;

    public MqttSistemaSubscriber(Optional<Mqtt5BlockingClient> mqttClient) {
        this.mqttClient = mqttClient.orElse(null);
    }

    @PostConstruct
    public void startSubscription() {
        if (mqttClient == null) {
            log.warn("MQTT no disponible, no se suscribirá a eventos de sistema");
            return;
        }

        running = true;
        executor.submit(() -> {
            try {
                mqttClient.toAsync().subscribeWith()
                    .topicFilter("riego/+/sistema/evento")
                    .callback(publish -> {
                        try {
                            String topic = publish.getTopic().toString();
                            String payload = new String(publish.getPayloadAsBytes(), StandardCharsets.UTF_8);
                            handleSystemEvent(topic, payload);
                        } catch (Exception e) {
                            log.error("Error procesando mensaje MQTT sistema", e);
                        }
                    })
                    .send();

                log.info("Suscrito a topics MQTT de sistema: riego/+/sistema/evento");

                while (running) {
                    Thread.sleep(1000);
                }
            } catch (InterruptedException e) {
                Thread.currentThread().interrupt();
                log.info("Suscripción MQTT sistema interrumpida");
            } catch (Exception e) {
                log.error("Error en suscripción MQTT sistema", e);
            }
        });
    }

    @PreDestroy
    public void stopSubscription() {
        running = false;
        executor.shutdownNow();
    }

    private void handleSystemEvent(String topic, String payload) {
        try {
            // Topic format: riego/{nodeId}/sistema/evento
            String[] parts = topic.split("/");
            if (parts.length < 3) {
                log.warn("Formato de topic inválido: {}", topic);
                return;
            }

            UUID nodeId = UUID.fromString(parts[1]);

            JsonNode json = objectMapper.readTree(payload);
            if (!json.hasNonNull("tipo")) {
                log.warn("Evento de sistema sin tipo, se descarta: {}", payload);
                return;
            }

            String tipo = json.get("tipo").asText();
            String detalles = json.has("detalles") && !json.get("detalles").isNull()
                    ? json.get("detalles").asText()
                    : "(sin detalles)";

            // Log según severidad
            switch (tipo) {
                case "agenda_parse_error":
                case "agenda_format_error":
                case "agenda_storage_error":
                case "agenda_load_error":
                    log.error("[Sistema] nodeId={} tipo={} detalles={}", nodeId, tipo, detalles);
                    break;
                case "agenda_fetch_warning":
                    log.warn("[Sistema] nodeId={} tipo={} detalles={}", nodeId, tipo, detalles);
                    break;
                default:
                    log.info("[Sistema] nodeId={} tipo={} detalles={}", nodeId, tipo, detalles);
                    break;
            }

        } catch (Exception e) {
            log.error("Error parseando evento de sistema: payload={} error={}", payload, e.getMessage());
        }
    }
}
