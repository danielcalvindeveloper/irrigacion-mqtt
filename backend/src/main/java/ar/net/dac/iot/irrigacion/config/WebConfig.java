package ar.net.dac.iot.irrigacion.config;

import org.springframework.context.annotation.Bean;
import org.springframework.context.annotation.Configuration;
import org.springframework.core.io.ClassPathResource;
import org.springframework.core.io.Resource;
import org.springframework.web.servlet.config.annotation.CorsRegistry;
import org.springframework.web.servlet.config.annotation.ResourceHandlerRegistry;
import org.springframework.web.servlet.config.annotation.WebMvcConfigurer;
import org.springframework.web.servlet.resource.PathResourceResolver;

import java.io.IOException;
import java.time.Clock;
import java.time.ZoneId;

/**
 * Configuración web para:
 * 1. CORS (desarrollo con frontend separado)
 * 2. Servir frontend Vue.js embebido (producción)
 * 3. Proveer Clock del sistema para inyección de dependencias
 */
@Configuration
public class WebConfig implements WebMvcConfigurer {

@Bean
public Clock clock() {
    return Clock.system(ZoneId.of("America/Argentina/Buenos_Aires"));
}

@Override
public void addCorsMappings(CorsRegistry registry) {
    // CORS para desarrollo con frontend en puerto 5173 y producción con Cloudflare
    registry.addMapping("/**")  // Permitir CORS en todas las rutas (no solo /api/**)
            .allowedOriginPatterns(
                "http://localhost:*",           // Localhost cualquier puerto
                "http://127.0.0.1:*",
                "http://192.168.*.*:*",         // Red local 192.168.x.x
                "http://10.*.*.*:*",            // Red local 10.x.x.x
                "https://*.dac.net.ar",         // Dominio de producción con Cloudflare
                "https://apiriego.dac.net.ar"   // URL específica de producción
            )
            .allowedMethods("GET", "POST", "PUT", "DELETE", "PATCH", "OPTIONS")
            .allowedHeaders("*")
            .exposedHeaders("*")
            .allowCredentials(true)  // CRÍTICO: permite enviar credenciales de autenticación
            .maxAge(3600);
}

@Override
public void addResourceHandlers(ResourceHandlerRegistry registry) {
    // Servir archivos estáticos del frontend embebido (JS, CSS, imágenes)
    registry.addResourceHandler("/assets/**")
            .addResourceLocations("classpath:/static/assets/");

    // Servir index.html para todas las rutas no API (SPA routing)
    // Esto permite que Vue Router maneje el enrutamiento del lado del cliente
    registry.addResourceHandler("/**")
            .addResourceLocations("classpath:/static/")
            .resourceChain(true)
            .addResolver(new PathResourceResolver() {
                @Override
                protected Resource getResource(String resourcePath, Resource location) throws IOException {
                    Resource requestedResource = location.createRelative(resourcePath);
                    
                    // Si el recurso existe, devolverlo (archivos estáticos)
                    if (requestedResource.exists() && requestedResource.isReadable()) {
                        return requestedResource;
                    }
                    
                    // Si no existe y NO es una ruta API, devolver index.html para SPA routing
                    if (!resourcePath.startsWith("api/")) {
                        return new ClassPathResource("/static/index.html");
                    }
                    
                    return null;
                }
            });
}
}
