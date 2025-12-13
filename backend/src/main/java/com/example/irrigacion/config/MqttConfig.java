package com.example.irrigacion.config;

import com.hivemq.client.mqtt.MqttClient;
import com.hivemq.client.mqtt.mqtt5.Mqtt5BlockingClient;
import com.hivemq.client.mqtt.mqtt5.Mqtt5ClientBuilder;
import com.hivemq.client.mqtt.mqtt5.message.connect.connack.Mqtt5ConnAck;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.boot.autoconfigure.condition.ConditionalOnProperty;
import org.springframework.boot.context.properties.EnableConfigurationProperties;
import org.springframework.context.annotation.Bean;
import org.springframework.context.annotation.Configuration;

import java.nio.charset.StandardCharsets;
import java.util.Optional;
import java.util.UUID;

@Configuration
@EnableConfigurationProperties(MqttProperties.class)
@ConditionalOnProperty(prefix = "app.mqtt", name = "enabled", havingValue = "true")
public class MqttConfig {
    private static final Logger log = LoggerFactory.getLogger(MqttConfig.class);

    @Bean
    public Mqtt5BlockingClient mqttClient(MqttProperties props) {
        if (props.getHost() == null || props.getHost().isBlank()) {
            throw new IllegalStateException("app.mqtt.host no configurado");
        }

        String clientId = Optional.ofNullable(props.getClientId())
                .filter(id -> !id.isBlank())
                .orElse("backend-" + UUID.randomUUID());

        Mqtt5ClientBuilder builder = MqttClient.builder()
            .useMqttVersion5()
            .identifier(clientId)
            .serverHost(props.getHost())
            .serverPort(props.getPort());

        if (props.isTls()) {
            builder = builder.sslWithDefaultConfig();
        }

        Mqtt5BlockingClient client = builder.buildBlocking();

        var connect = client.connectWith().cleanStart(true);
        if (props.getUsername() != null && !props.getUsername().isBlank()) {
            connect.simpleAuth()
                    .username(props.getUsername())
                    .password(props.getPassword() == null ? new byte[0] : props.getPassword().getBytes(StandardCharsets.UTF_8))
                    .applySimpleAuth();
        }

        Mqtt5ConnAck ack = connect.send();

        log.info("MQTT conectado a {}:{} (tls={}) result={}", props.getHost(), props.getPort(), props.isTls(), ack.getReasonCode());
        return client;
    }
}
