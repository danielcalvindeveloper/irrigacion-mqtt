package ar.net.dac.iot.irrigacion;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.beans.factory.annotation.Value;
import org.springframework.boot.CommandLineRunner;
import org.springframework.boot.SpringApplication;
import org.springframework.boot.autoconfigure.SpringBootApplication;
import org.springframework.context.annotation.Bean;

@SpringBootApplication
public class IrrigacionApplication {
    
    private static final Logger log = LoggerFactory.getLogger(IrrigacionApplication.class);

    @Value("${app.version:UNKNOWN}")
    private String version;

    @Value("${app.build-timestamp:UNKNOWN}")
    private String buildTimestamp;

    public static void main(String[] args) {
        SpringApplication.run(IrrigacionApplication.class, args);
    }

    @Bean
    public CommandLineRunner logVersion() {
        return args -> {
            log.info("═══════════════════════════════════════════════════════");
            log.info("  Irrigación Backend - Version: {}", version);
            log.info("  Build Timestamp: {}", buildTimestamp);
            log.info("═══════════════════════════════════════════════════════");
        };
    }
}
