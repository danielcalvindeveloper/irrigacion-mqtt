package com.example.irrigacion;

import com.example.irrigacion.dto.AgendaRequest;
import com.example.irrigacion.dto.AgendaResponse;
import com.example.irrigacion.service.AgendaService;
import com.hivemq.client.mqtt.MqttGlobalPublishFilter;
import com.hivemq.client.mqtt.mqtt5.Mqtt5BlockingClient;
import com.hivemq.client.mqtt.mqtt5.Mqtt5Client;
import org.junit.jupiter.api.BeforeEach;
import org.junit.jupiter.api.Test;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.boot.test.context.SpringBootTest;
import org.springframework.test.context.DynamicPropertyRegistry;
import org.springframework.test.context.DynamicPropertySource;
import org.testcontainers.containers.GenericContainer;
import org.testcontainers.containers.PostgreSQLContainer;
import org.testcontainers.junit.jupiter.Container;
import org.testcontainers.junit.jupiter.Testcontainers;

import java.nio.charset.StandardCharsets;
import java.util.List;
import java.util.UUID;
import java.util.concurrent.TimeUnit;

import static org.junit.jupiter.api.Assertions.*;

@SpringBootTest
@Testcontainers
class MqttIntegrationTest {

    static {
        System.setProperty("user.timezone", "UTC");
        java.util.TimeZone.setDefault(java.util.TimeZone.getTimeZone("UTC"));
    }

    @Container
    static PostgreSQLContainer<?> postgres = new PostgreSQLContainer<>("postgres:15")
            .withEnv("TZ", "UTC")
            .withUrlParam("options", "-c TimeZone=UTC");

    // HiveMQ CE container exposing MQTT on 1883 (no TLS in tests)
    @Container
    static GenericContainer<?> mqtt = new GenericContainer<>("hivemq/hivemq-ce:2023.5")
            .withExposedPorts(1883);

    @DynamicPropertySource
    static void registerProps(DynamicPropertyRegistry registry) {
        registry.add("spring.datasource.url", () -> postgres.getJdbcUrl() + "&options=-c%20TimeZone=UTC");
        registry.add("spring.datasource.username", postgres::getUsername);
        registry.add("spring.datasource.password", postgres::getPassword);
        registry.add("spring.datasource.driver-class-name", () -> "org.postgresql.Driver");
        registry.add("spring.flyway.enabled", () -> true);
        registry.add("spring.datasource.hikari.connection-init-sql", () -> "SET TIME ZONE 'UTC'");
        registry.add("spring.jpa.properties.hibernate.jdbc.time_zone", () -> "UTC");

        registry.add("app.mqtt.enabled", () -> true);
        registry.add("app.mqtt.host", () -> mqtt.getHost());
        registry.add("app.mqtt.port", () -> mqtt.getMappedPort(1883));
        registry.add("app.mqtt.tls", () -> false); // sin TLS en tests
    }

    @Autowired
    private org.springframework.context.ApplicationContext ctx;

    @Autowired
    private AgendaService agendaService;

    private com.hivemq.client.mqtt.mqtt5.Mqtt5AsyncClient testClient;
    private final java.util.concurrent.BlockingQueue<String> receivedMessages = new java.util.concurrent.LinkedBlockingQueue<>();

    @BeforeEach
    void setupTestClient() {
        receivedMessages.clear();
        testClient = Mqtt5Client.builder()
                .serverHost(mqtt.getHost())
                .serverPort(mqtt.getMappedPort(1883))
                .buildAsync();
        testClient.connectWith().send().join();

        // Suscribir a todos los topics de riego para capturar publicaciones
        testClient.subscribeWith()
                .topicFilter("riego/#")
                .callback(publish -> {
                    String payload = new String(publish.getPayloadAsBytes(), StandardCharsets.UTF_8);
                    receivedMessages.offer(payload);
                })
                .send()
                .join();
    }

    @Test
    void contextStartsWithContainers() {
        // smoke: garantiza que el contexto sube con DB + broker mock
        assert ctx != null;
    }

    @Test
    void whenAgendaCreated_thenPublishesToMqtt() throws Exception {
        UUID nodeId = UUID.randomUUID();
        UUID agendaId = UUID.randomUUID();

        // Crear agenda
        AgendaRequest request = new AgendaRequest();
        request.setId(agendaId);
        request.setNodeId(nodeId);
        request.setZona((short) 1);
        request.setDiasSemana(List.of("LUN", "MIE"));
        request.setHoraInicio("08:00");
        request.setDuracionMin((short) 15);
        request.setActiva(true);

        AgendaResponse response = agendaService.upsert(request);

        // Esperar mensaje
        String payload = receivedMessages.poll(5, TimeUnit.SECONDS);
        assertNotNull(payload, "Debe publicar mensaje al crear agenda");
        assertTrue(payload.contains("\"version\":1"), "Payload debe incluir versión 1");
        assertTrue(payload.contains(agendaId.toString()), "Payload debe incluir agenda creada");
    }

    @Test
    void whenAgendaUpdated_thenIncrementsVersion() throws Exception {
        UUID nodeId = UUID.randomUUID();
        UUID agendaId = UUID.randomUUID();

        // Crear agenda inicial
        AgendaRequest request = new AgendaRequest();
        request.setId(agendaId);
        request.setNodeId(nodeId);
        request.setZona((short) 2);
        request.setDiasSemana(List.of("MAR"));
        request.setHoraInicio("06:30");
        request.setDuracionMin((short) 20);

        agendaService.upsert(request);
        String payload1 = receivedMessages.poll(3, TimeUnit.SECONDS); // consumir v1
        assertNotNull(payload1);

        // Actualizar agenda
        request.setDuracionMin((short) 30);
        agendaService.upsert(request);

        // Capturar mensaje de actualización
        String payload2 = receivedMessages.poll(3, TimeUnit.SECONDS);
        assertNotNull(payload2, "Debe publicar al actualizar");
        assertTrue(payload2.contains("\"version\":2"), "Versión debe incrementarse a 2");
    }
}
