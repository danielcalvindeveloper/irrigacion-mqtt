package ar.net.dac.iot.irrigacion.service;

import com.fasterxml.jackson.databind.ObjectMapper;
import com.hivemq.client.mqtt.mqtt5.Mqtt5BlockingClient;
import jakarta.annotation.PostConstruct;
import jakarta.annotation.PreDestroy;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.boot.autoconfigure.condition.ConditionalOnBean;
import org.springframework.stereotype.Service;

import java.nio.charset.StandardCharsets;
import java.util.Map;
import java.util.Optional;
import java.util.UUID;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

@Service
@ConditionalOnBean(Mqtt5BlockingClient.class)
public class MqttStatusSubscriber {
    private static final Logger log = LoggerFactory.getLogger(MqttStatusSubscriber.class);
    
    private final Mqtt5BlockingClient mqttClient;
    private final ZoneStatusService zoneStatusService;
    private final ObjectMapper objectMapper = new ObjectMapper();
    private final ExecutorService executor = Executors.newSingleThreadExecutor();
    private volatile boolean running = false;

    public MqttStatusSubscriber(Optional<Mqtt5BlockingClient> mqttClient, ZoneStatusService zoneStatusService) {
        this.mqttClient = mqttClient.orElse(null);
        this.zoneStatusService = zoneStatusService;
    }

    @PostConstruct
    public void startSubscription() {
        if (mqttClient == null) {
            log.warn("MQTT no disponible, no se suscribirá a status");
            return;
        }

        running = true;
        executor.submit(() -> {
            try {
                // Suscribirse a todos los topics de status usando publish callback
                mqttClient.toAsync().subscribeWith()
                    .topicFilter("riego/+/status/zona/+")
                    .callback(publish -> {
                        try {
                            String topic = publish.getTopic().toString();
                            String payload = new String(publish.getPayloadAsBytes(), StandardCharsets.UTF_8);
                            handleStatusMessage(topic, payload);
                        } catch (Exception e) {
                            log.error("Error procesando mensaje MQTT status", e);
                        }
                    })
                    .send();
                
                log.info("Suscrito a topics MQTT de status: riego/+/status/zona/+");
                
                // Mantener vivo
                while (running) {
                    Thread.sleep(1000);
                }
            } catch (InterruptedException e) {
                Thread.currentThread().interrupt();
                log.info("Suscripción MQTT interrumpida");
            } catch (Exception e) {
                log.error("Error en suscripción MQTT", e);
            }
        });
    }

    @PreDestroy
    public void stopSubscription() {
        running = false;
        executor.shutdownNow();
    }

    private void handleStatusMessage(String topic, String payload) {
        try {
            // Topic format: riego/{nodeId}/status/zona/{zona}
            String[] parts = topic.split("/");
            if (parts.length != 5) {
                log.warn("Formato de topic inválido: {}", topic);
                return;
            }

            UUID nodeId = UUID.fromString(parts[1]);
            int zona = Integer.parseInt(parts[4]);

            // Parse payload JSON: {"activa": true/false, "tiempoRestante": seconds}
            Map<String, Object> data = objectMapper.readValue(payload, Map.class);
            boolean activa = (boolean) data.getOrDefault("activa", false);
            Integer tiempoRestante = data.containsKey("tiempoRestante") ? 
                ((Number) data.get("tiempoRestante")).intValue() : 0;

            zoneStatusService.updateZoneStatus(nodeId, zona, activa, tiempoRestante);
            
            log.debug("Status actualizado: node={} zona={} activa={} tiempo={}", 
                nodeId, zona, activa, tiempoRestante);

        } catch (Exception e) {
            log.error("Error parseando mensaje status: topic={} payload={}", topic, payload, e);
        }
    }
}
