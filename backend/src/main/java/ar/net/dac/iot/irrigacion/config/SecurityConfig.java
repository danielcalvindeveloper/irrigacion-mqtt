package ar.net.dac.iot.irrigacion.config;

import org.springframework.beans.factory.annotation.Value;
import org.springframework.context.annotation.Bean;
import org.springframework.context.annotation.Configuration;
import org.springframework.security.config.Customizer;
import org.springframework.security.config.annotation.web.builders.HttpSecurity;
import org.springframework.security.config.annotation.web.configuration.EnableWebSecurity;
import org.springframework.security.core.userdetails.User;
import org.springframework.security.core.userdetails.UserDetails;
import org.springframework.security.core.userdetails.UserDetailsService;
import org.springframework.security.crypto.bcrypt.BCryptPasswordEncoder;
import org.springframework.security.crypto.password.PasswordEncoder;
import org.springframework.security.provisioning.InMemoryUserDetailsManager;
import org.springframework.security.web.SecurityFilterChain;

/**
 * Configuración de seguridad con HTTP Basic Auth.
 * 
 * Para acceso público a internet, toda la aplicación (frontend + APIs)
 * requiere autenticación con usuario y contraseña.
 * 
 * Credenciales configurables por variables de entorno:
 * - SECURITY_USERNAME (default: admin)
 * - SECURITY_PASSWORD (default: cambiar_en_produccion)
 */
@Configuration
@EnableWebSecurity
public class SecurityConfig {

    @Value("${app.security.username:admin}")
    private String username;
    
    @Value("${app.security.password:cambiar_en_produccion}")
    private String password;

    @Bean
    public SecurityFilterChain securityFilterChain(HttpSecurity http) throws Exception {
        http
            .csrf(csrf -> csrf.disable())  // Deshabilitar CSRF para REST API
            .cors(Customizer.withDefaults())  // Habilitar CORS (ya configurado en WebConfig)
            .authorizeHttpRequests(auth -> auth
                .anyRequest().authenticated()  // Todo requiere autenticación
            )
            .httpBasic(basic -> basic
                .realmName("Irrigacion System")  // Nombre mostrado en popup de autenticación
            );
        
        return http.build();
    }

    @Bean
    public UserDetailsService userDetailsService() {
        UserDetails user = User.builder()
            .username(username)
            .password(passwordEncoder().encode(password))
            .roles("USER")
            .build();
        
        return new InMemoryUserDetailsManager(user);
    }

    @Bean
    public PasswordEncoder passwordEncoder() {
        return new BCryptPasswordEncoder();
    }
}
