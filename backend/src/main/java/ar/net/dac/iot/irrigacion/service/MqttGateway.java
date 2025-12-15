package ar.net.dac.iot.irrigacion.service;

import ar.net.dac.iot.irrigacion.config.MqttProperties;
import com.hivemq.client.mqtt.datatypes.MqttQos;
import com.hivemq.client.mqtt.mqtt5.Mqtt5BlockingClient;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.boot.autoconfigure.condition.ConditionalOnProperty;
import org.springframework.stereotype.Service;

import jakarta.annotation.PreDestroy;

@Service
@ConditionalOnProperty(prefix = "app.mqtt", name = "enabled", havingValue = "true")
public class MqttGateway {
    private static final Logger log = LoggerFactory.getLogger(MqttGateway.class);

    private final Mqtt5BlockingClient client;
    private final MqttProperties props;

    public MqttGateway(Mqtt5BlockingClient client, MqttProperties props) {
        this.client = client;
        this.props = props;
    }

    /**
     * Publica un payload en un topic con QoS 1.
     */
    public void publish(String topic, byte[] payload) {
        client.publishWith()
                .topic(topic)
                .qos(MqttQos.AT_LEAST_ONCE)
                .payload(payload)
                .send();
    }

    /**
     * Conveniencia para comandos manuales ON/OFF.
     */
    public void publishCommand(String nodeId, int zona, String accion, Integer duracionSeg) {
        String topic = String.format("riego/%s/cmd/zona/%d", nodeId, zona);
        String json = duracionSeg == null
                ? String.format("{\"accion\":\"%s\"}", accion)
                : String.format("{\"accion\":\"%s\",\"duracion\":%d}", accion, duracionSeg);
        publish(topic, json.getBytes());
        log.info("MQTT cmd publicado topic={} accion={} duracion={}", topic, accion, duracionSeg);
    }

    @PreDestroy
    public void shutdown() {
        try {
            client.disconnect();
            log.info("MQTT desconectado");
        } catch (Exception e) {
            log.warn("Error al desconectar MQTT", e);
        }
    }

    public boolean isEnabled() {
        return props.isEnabled();
    }
}
