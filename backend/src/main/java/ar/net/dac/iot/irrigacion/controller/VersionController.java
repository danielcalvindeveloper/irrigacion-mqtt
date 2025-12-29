package ar.net.dac.iot.irrigacion.controller;

import org.springframework.beans.factory.annotation.Value;
import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.RequestMapping;
import org.springframework.web.bind.annotation.RestController;

import java.util.HashMap;
import java.util.Map;

/**
 * Controlador para exponer información de versión y build del backend
 */
@RestController
@RequestMapping("/api/version")
public class VersionController {

    @Value("${app.version:UNKNOWN}")
    private String version;

    @Value("${app.build-timestamp:UNKNOWN}")
    private String buildTimestamp;

    @GetMapping
    public Map<String, String> getVersion() {
        Map<String, String> versionInfo = new HashMap<>();
        versionInfo.put("version", version);
        versionInfo.put("buildTimestamp", buildTimestamp);
        versionInfo.put("application", "irrigacion-backend");
        return versionInfo;
    }
}
