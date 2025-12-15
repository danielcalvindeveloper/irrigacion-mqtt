# Stage 1: Build frontend
FROM node:18-alpine AS frontend-build
WORKDIR /frontend
COPY frontend/package*.json ./
RUN npm ci
COPY frontend/ ./
RUN npm run build

# Stage 2: Build backend
FROM eclipse-temurin:17-jdk-alpine AS backend-build
WORKDIR /app
COPY backend/pom.xml .
COPY backend/src ./src
# Copiar frontend build a recursos estáticos de Spring Boot
COPY --from=frontend-build /frontend/dist ./src/main/resources/static
RUN apk add --no-cache maven && mvn clean package -DskipTests

# Stage 3: Runtime
FROM eclipse-temurin:17-jre-alpine
WORKDIR /app
COPY --from=backend-build /app/target/irrigacion-*.jar app.jar
ENV JAVA_OPTS="-Duser.timezone=UTC"
EXPOSE 8080
ENTRYPOINT ["sh", "-c", "java $JAVA_OPTS -jar app.jar"]
