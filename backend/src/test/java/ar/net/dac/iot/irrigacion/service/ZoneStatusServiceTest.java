package ar.net.dac.iot.irrigacion.service;

import ar.net.dac.iot.irrigacion.dto.ZoneStatusResponse;
import ar.net.dac.iot.irrigacion.model.Agenda;
import ar.net.dac.iot.irrigacion.repository.AgendaRepository;
import org.junit.jupiter.api.BeforeEach;
import org.junit.jupiter.api.Test;
import org.junit.jupiter.api.extension.ExtendWith;
import org.mockito.InjectMocks;
import org.mockito.Mock;
import org.mockito.junit.jupiter.MockitoExtension;
import org.mockito.junit.jupiter.MockitoSettings;
import org.mockito.quality.Strictness;

import java.time.Clock;
import java.time.DayOfWeek;
import java.time.Instant;
import java.time.LocalDate;
import java.time.LocalTime;
import java.time.ZoneId;
import java.util.Arrays;
import java.util.Collections;
import java.util.List;
import java.util.UUID;

import static org.junit.jupiter.api.Assertions.*;
import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.anyShort;
import static org.mockito.Mockito.when;

@ExtendWith(MockitoExtension.class)
@MockitoSettings(strictness = Strictness.LENIENT)
class ZoneStatusServiceTest {

    @Mock
    private AgendaRepository agendaRepository;

    @Mock
    private ZoneConfigService zoneConfigService;

    @Mock
    private Clock clock;

    @InjectMocks
    private ZoneStatusService zoneStatusService;

    private UUID testNodeId;
    private ZoneId zoneId;
    private static final short TEST_ZONA = 1;

    @BeforeEach
    void setUp() {
        testNodeId = UUID.randomUUID();
        zoneId = ZoneId.of("America/Argentina/Buenos_Aires");
        // Configurar para que getStatus devuelva lista vacía por defecto
        when(zoneConfigService.listEnabledByNode(any())).thenReturn(Collections.emptyList());
    }

    /**
     * Helper para mockear el tiempo actual del sistema
     */
    private void mockCurrentTime(LocalDate date, LocalTime time) {
        Instant instant = date.atTime(time).atZone(zoneId).toInstant();
        when(clock.instant()).thenReturn(instant);
        when(clock.getZone()).thenReturn(zoneId);
    }

    @Test
    void testCalcularProximoRiego_sinAgendas() {
        // Given: No hay agendas
        when(agendaRepository.findActiveByNodeAndZona(testNodeId, TEST_ZONA))
            .thenReturn(Collections.emptyList());

        // When: Se pide el estado
        List<ZoneStatusResponse> result = zoneStatusService.getStatus(testNodeId);

        // Then: No hay próximo riego
        // Como no hay zonas configuradas, la lista está vacía
        assertTrue(result.isEmpty());
    }

    @Test
    void testCalcularProximoRiego_agendaHoyFutura() {
        // Given: Agenda para hoy a las 23:59 (asumiendo que ahora es antes)
        LocalDate hoy = LocalDate.of(2025, 12, 27); // Sábado
        mockCurrentTime(hoy, LocalTime.of(10, 0)); // Mock: 10:00 AM
        
        Agenda agenda = crearAgenda(
            "Test Hoy",
            TEST_ZONA,
            LocalTime.of(23, 59),
            10,
            "SAB"
        );

        when(agendaRepository.findActiveByNodeAndZona(testNodeId, TEST_ZONA))
            .thenReturn(List.of(agenda));

        // When: Se calcula próximo riego
        String resultado = getProximoRiegoForZona(TEST_ZONA);

        // Then: Debe ser "Hoy 23:59"
        assertNotNull(resultado);
        assertTrue(resultado.startsWith("Hoy 23:59"));
    }

    @Test
    void testCalcularProximoRiego_agendaHoyPasada() {
        // Given: Agenda para hoy a las 00:01 (ya pasó)
        LocalDate hoy = LocalDate.of(2025, 12, 28); // Sábado
        mockCurrentTime(hoy, LocalTime.of(10, 0)); // Mock: 10:00 AM (después de 00:01)
        
        Agenda agenda = crearAgenda(
            "Test Pasada",
            TEST_ZONA,
            LocalTime.of(0, 1),
            5,
            "SAB", "DOM"  // Incluir domingo para que encuentre próxima ocurrencia
        );

        when(agendaRepository.findActiveByNodeAndZona(testNodeId, TEST_ZONA))
            .thenReturn(List.of(agenda));

        // When: Se calcula próximo riego
        String resultado = getProximoRiegoForZona(TEST_ZONA);

        // Then: Debe ser para mañana (domingo) porque ya pasó la hora
        assertNotNull(resultado);
        assertFalse(resultado.startsWith("Hoy"), 
            "No debería decir 'Hoy' porque la hora ya pasó");
    }

    @Test
    void testCalcularProximoRiego_multipleAgendasEligeMasCercana() {
        // Given: 3 agendas en diferentes días
        LocalDate hoy = LocalDate.now();
        
        // Agenda 1: Dentro de 3 días
        DayOfWeek dia1 = hoy.plusDays(3).getDayOfWeek();
        Agenda agenda1 = crearAgenda(
            "Agenda +3 días",
            TEST_ZONA,
            LocalTime.of(10, 0),
            15,
            getDiaSemanaString(dia1)
        );
        
        // Agenda 2: Mañana (más cercana)
        DayOfWeek dia2 = hoy.plusDays(1).getDayOfWeek();
        Agenda agenda2 = crearAgenda(
            "Agenda Mañana",
            TEST_ZONA,
            LocalTime.of(8, 0),
            10,
            getDiaSemanaString(dia2)
        );
        
        // Agenda 3: Dentro de 5 días
        DayOfWeek dia3 = hoy.plusDays(5).getDayOfWeek();
        Agenda agenda3 = crearAgenda(
            "Agenda +5 días",
            TEST_ZONA,
            LocalTime.of(18, 30),
            20,
            getDiaSemanaString(dia3)
        );

        when(agendaRepository.findActiveByNodeAndZona(testNodeId, TEST_ZONA))
            .thenReturn(Arrays.asList(agenda1, agenda2, agenda3));

        // When: Se calcula próximo riego
        String resultado = getProximoRiegoForZona(TEST_ZONA);

        // Then: Debe elegir la de mañana
        assertNotNull(resultado);
        assertTrue(resultado.startsWith("Mañana 08:00"), 
            "Debería elegir la agenda de mañana. Resultado: " + resultado);
        assertTrue(resultado.contains("(10min)"));
    }

    @Test
    void testCalcularProximoRiego_mismoDiaDiferentesHoras() {
        // Given: 2 agendas para mañana a diferentes horas
        LocalDate hoy = LocalDate.of(2025, 12, 27); // Viernes
        mockCurrentTime(hoy, LocalTime.of(10, 0));
        
        Agenda agenda1 = crearAgenda(
            "Agenda 18:00",
            TEST_ZONA,
            LocalTime.of(18, 0),
            15,
            "SAB"  // Mañana
        );
        
        Agenda agenda2 = crearAgenda(
            "Agenda 06:00",
            TEST_ZONA,
            LocalTime.of(6, 0),
            10,
            "SAB"  // Mañana
        );

        when(agendaRepository.findActiveByNodeAndZona(testNodeId, TEST_ZONA))
            .thenReturn(Arrays.asList(agenda1, agenda2));

        // When: Se calcula próximo riego
        String resultado = getProximoRiegoForZona(TEST_ZONA);

        // Then: Debe elegir la más temprana (06:00)
        assertNotNull(resultado);
        assertTrue(resultado.startsWith("Mañana 06:00"), 
            "Debería elegir la agenda más temprana del día. Resultado: " + resultado);
    }

    @Test
    void testCalcularProximoRiego_agendaTodosLosDias() {
        // Given: Agenda que se ejecuta todos los días a las 19:00
        LocalDate hoy = LocalDate.of(2025, 12, 29);
        mockCurrentTime(hoy, LocalTime.of(10, 0)); // Mock: 10:00 AM (antes de las 19:00)
        
        Agenda agenda = crearAgenda(
            "Diaria 19:00",
            TEST_ZONA,
            LocalTime.of(19, 0),
            10,
            "LUN", "MAR", "MIE", "JUE", "VIE", "SAB", "DOM"
        );

        when(agendaRepository.findActiveByNodeAndZona(testNodeId, TEST_ZONA))
            .thenReturn(List.of(agenda));

        // When: Se calcula próximo riego
        String resultado = getProximoRiegoForZona(TEST_ZONA);

        // Then: Debe ser hoy porque son las 10:00
        assertNotNull(resultado);
        assertTrue(resultado.startsWith("Hoy 19:00"), 
            "A las 10:00, debe decir 'Hoy'. Resultado: " + resultado);
    }

    @Test
    void testCalcularProximoRiego_agendaSoloUnDiaRemoto() {
        // Given: Agenda solo para dentro de 7 días (límite del algoritmo)
        LocalDate hoy = LocalDate.of(2025, 12, 21); // Domingo
        mockCurrentTime(hoy, LocalTime.of(10, 0));
        // 7 días después: Domingo 28/12
        
        Agenda agenda = crearAgenda(
            "Agenda Remota",
            TEST_ZONA,
            LocalTime.of(12, 0),
            30,
            "DOM"  // Próximo domingo
        );

        when(agendaRepository.findActiveByNodeAndZona(testNodeId, TEST_ZONA))
            .thenReturn(List.of(agenda));

        // When: Se calcula próximo riego
        String resultado = getProximoRiegoForZona(TEST_ZONA);

        // Then: Debe encontrar la fecha (formato dd/MM)
        assertNotNull(resultado);
        assertTrue(resultado.contains("28/12"), 
            "Debe mostrar la fecha del próximo domingo (28/12). Resultado: " + resultado);
    }

    @Test
    void testUpdateZoneStatus() {
        // Given: Una zona sin estado previo
        UUID nodeId = UUID.randomUUID();
        int zona = 2;

        // When: Se actualiza el estado
        zoneStatusService.updateZoneStatus(nodeId, zona, true, 300);

        // Then: El estado se guarda en memoria (validación indirecta)
        // No hay getter público, pero podemos verificar que no lanza excepción
        assertDoesNotThrow(() -> 
            zoneStatusService.updateZoneStatus(nodeId, zona, false, 0)
        );
    }

    // --- Helpers ---

    private Agenda crearAgenda(String nombre, short zona, LocalTime hora, int duracion, String... dias) {
        Agenda agenda = new Agenda();
        agenda.setId(UUID.randomUUID());
        agenda.setNodeId(testNodeId);
        agenda.setNombre(nombre);
        agenda.setZona(zona);
        agenda.setHoraInicio(hora);
        agenda.setDuracionMin((short) duracion);
        agenda.setDiasSemana(List.of(dias));
        agenda.setActiva(true);
        agenda.setVersion(1);
        return agenda;
    }

    private String getDiaSemanaString(DayOfWeek dayOfWeek) {
        return switch (dayOfWeek) {
            case MONDAY -> "LUN";
            case TUESDAY -> "MAR";
            case WEDNESDAY -> "MIE";
            case THURSDAY -> "JUE";
            case FRIDAY -> "VIE";
            case SATURDAY -> "SAB";
            case SUNDAY -> "DOM";
        };
    }

    /**
     * Método helper que simula la llamada interna a calcularProximoRiego
     * ya que es privado. En su lugar, usamos getStatus con mocking apropiado.
     */
    private String getProximoRiegoForZona(short zona) {
        // Crear una zona config mock para que getStatus devuelva algo
        var zoneConfig = new ar.net.dac.iot.irrigacion.dto.ZoneConfigResponse();
        zoneConfig.setZona(zona);
        zoneConfig.setNombre("Zona Test");
        zoneConfig.setHabilitada(true);
        
        when(zoneConfigService.listEnabledByNode(testNodeId))
            .thenReturn(List.of(zoneConfig));
        
        List<ZoneStatusResponse> status = zoneStatusService.getStatus(testNodeId);
        
        if (status.isEmpty()) {
            return null;
        }
        
        return status.get(0).getProximoRiego();
    }

    @Test
    void testEscenarioRealDomingo11h48() {
        // ESCENARIO REAL del usuario:
        // Hoy: Domingo 28/12/2025 11:48
        // Zona 3 tiene dos agendas:
        // 1. "Prueba 2": DOM/SAB/MAR/JUE a las 11:55
        // 2. "Alguna": SAB/DOM a las 20:20
        // Resultado esperado: "Hoy 11:55 (10min)" porque 11:55 está más cerca (7 min) que 20:20 (8.5 horas)
        
        // Mock del tiempo: Domingo 28/12/2025 a las 11:48
        LocalDate domingo = LocalDate.of(2025, 12, 28);
        LocalTime las11y48 = LocalTime.of(11, 48);
        mockCurrentTime(domingo, las11y48);
        
        Agenda agenda1 = crearAgenda("Prueba 2", (short) 3, LocalTime.of(11, 55), 10, "DOM", "SAB", "MAR", "JUE");
        Agenda agenda2 = crearAgenda("Alguna", (short) 3, LocalTime.of(20, 20), 15, "SAB", "DOM");
        
        when(agendaRepository.findActiveByNodeAndZona(testNodeId, (short) 3))
            .thenReturn(Arrays.asList(agenda1, agenda2));
        
        String proximoRiego = getProximoRiegoForZona((short) 3);
        
        assertNotNull(proximoRiego, "Debe haber un próximo riego");
        assertTrue(proximoRiego.contains("11:55"), 
            "A las 11:48 del domingo, debe elegir 11:55 (7 min) no 20:20 (8.5 horas). Actual: " + proximoRiego);
        assertTrue(proximoRiego.contains("Hoy"), 
            "Debe mostrar 'Hoy' ya que es el mismo día. Actual: " + proximoRiego);
    }
}
