package ar.net.dac.iot.irrigacion.service;

import com.hivemq.client.mqtt.mqtt5.Mqtt5BlockingClient;
import com.hivemq.client.mqtt.mqtt5.message.publish.Mqtt5Publish;
import com.hivemq.client.mqtt.MqttGlobalPublishFilter;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.boot.context.event.ApplicationReadyEvent;
import org.springframework.context.event.EventListener;
import org.springframework.stereotype.Service;

import java.nio.charset.StandardCharsets;
import java.util.UUID;

@Service
public class MqttEventoSubscriber {
    
    private static final Logger log = LoggerFactory.getLogger(MqttEventoSubscriber.class);
    private final Mqtt5BlockingClient mqttClient;
    private final RiegoEventoService riegoEventoService;
    private volatile boolean running = false;

    public MqttEventoSubscriber(Mqtt5BlockingClient mqttClient, RiegoEventoService riegoEventoService) {
        this.mqttClient = mqttClient;
        this.riegoEventoService = riegoEventoService;
    }

    @EventListener(ApplicationReadyEvent.class)
    public void subscribeToEventos() {
        String topicFilter = "riego/+/evento";
        
        // Thread para escuchar mensajes en background
        Thread subscriberThread = new Thread(() -> {
            try {
                // Suscribirse al topic
                mqttClient.subscribeWith()
                    .topicFilter(topicFilter)
                    .send();
                
                log.info("Suscrito a topic MQTT de eventos: {}", topicFilter);
                running = true;
                
                // Loop infinito recibiendo mensajes
                try (var publishes = mqttClient.publishes(MqttGlobalPublishFilter.ALL)) {
                    while (true) {
                        try {
                            // receive() bloquea hasta que llega un mensaje y retorna Mqtt5Publish
                            Mqtt5Publish publish = publishes.receive();
                            if (publish == null) continue;
                            String topic = publish.getTopic().toString();
                            String payload = new String(
                                publish.getPayloadAsBytes(),
                                StandardCharsets.UTF_8
                            );

                            // Extraer nodeId del topic: riego/{nodeId}/evento
                            String[] parts = topic.split("/");
                            if (parts.length >= 2) {
                                UUID nodeId = UUID.fromString(parts[1]);
                                riegoEventoService.procesarEvento(nodeId, payload);
                            }

                        } catch (Exception e) {
                            log.error("Error procesando mensaje MQTT de evento: {}", e.getMessage(), e);
                        }
                    }
                }
                
            } catch (Exception e) {
                log.error("Error en subscriber de eventos de riego: {}", e.getMessage(), e);
                running = false;
            }
        }, "mqtt-evento-subscriber");
        
        subscriberThread.setDaemon(true);
        subscriberThread.start();
    }
}
