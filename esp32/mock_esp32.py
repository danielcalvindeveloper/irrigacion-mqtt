#!/usr/bin/env python3
"""
Mock ESP32 - Simulador de nodo de riego para testing

Simula un ESP32 conectado al broker MQTT:
- Se suscribe a riego/{nodeId}/agenda/sync para recibir agendas
- Se suscribe a riego/{nodeId}/cmd/zona/# para comandos manuales
- Simula la ejecución de riegos (logs en consola)
- Mantiene un estado simple de la agenda actual

Uso:
    python mock_esp32.py --node-id UUID_DEL_NODO [--mqtt-host localhost] [--mqtt-port 1883]
"""

import argparse
import json
import sys
import time
from datetime import datetime
from typing import Optional, Dict, Any

try:
    import paho.mqtt.client as mqtt
except ImportError:
    print("ERROR: paho-mqtt no está instalado.")
    print("Instala con: pip install paho-mqtt")
    sys.exit(1)


class MockESP32:
    """Simulador de nodo ESP32 de riego"""
    
    def __init__(self, node_id: str, mqtt_host: str = "localhost", mqtt_port: int = 1883):
        self.node_id = node_id
        self.mqtt_host = mqtt_host
        self.mqtt_port = mqtt_port
        self.client: Optional[mqtt.Client] = None
        self.agenda_actual: Optional[Dict[str, Any]] = None
        self.agenda_version: int = 0
        
        # Topics
        self.topic_agenda_sync = f"riego/{node_id}/agenda/sync"
        self.topic_cmd_zona = f"riego/{node_id}/cmd/zona/+"
        
    def on_connect(self, client, userdata, flags, rc, properties=None):
        """Callback cuando se conecta al broker"""
        if rc == 0:
            print(f"✓ Conectado al broker MQTT en {self.mqtt_host}:{self.mqtt_port}")
            print(f"✓ Node ID: {self.node_id}")
            
            # Suscribirse a los topics
            client.subscribe(self.topic_agenda_sync)
            print(f"✓ Suscrito a: {self.topic_agenda_sync}")
            
            client.subscribe(self.topic_cmd_zona)
            print(f"✓ Suscrito a: {self.topic_cmd_zona}")
            print("\n--- Esperando mensajes MQTT ---\n")
        else:
            print(f"✗ Error de conexión. Código: {rc}")
            
    def on_message(self, client, userdata, msg):
        """Callback cuando llega un mensaje MQTT"""
        topic = msg.topic
        timestamp = datetime.now().strftime("%H:%M:%S")
        
        try:
            payload = json.loads(msg.payload.decode())
            
            if topic == self.topic_agenda_sync:
                self._handle_agenda_sync(payload, timestamp)
            elif topic.startswith(f"riego/{self.node_id}/cmd/zona/"):
                zona = topic.split("/")[-1]
                self._handle_comando_zona(zona, payload, timestamp)
            else:
                print(f"[{timestamp}] Mensaje en topic desconocido: {topic}")
                
        except json.JSONDecodeError:
            print(f"[{timestamp}] ✗ Error: payload no es JSON válido")
        except Exception as e:
            print(f"[{timestamp}] ✗ Error procesando mensaje: {e}")
            
    def _handle_agenda_sync(self, payload: Dict[str, Any], timestamp: str):
        """Procesa un mensaje de sincronización de agenda"""
        print(f"\n{'='*60}")
        print(f"[{timestamp}] 📅 AGENDA RECIBIDA")
        print(f"{'='*60}")
        
        version = payload.get("version", 0)
        nombre = payload.get("nombre", "Sin nombre")
        activa = payload.get("activa", False)
        programaciones = payload.get("programaciones", [])
        
        print(f"Versión: {version}")
        print(f"Nombre: {nombre}")
        print(f"Activa: {'SÍ' if activa else 'NO'}")
        print(f"Programaciones: {len(programaciones)}")
        
        if self.agenda_version > 0 and version <= self.agenda_version:
            print(f"⚠️  Versión recibida ({version}) <= versión actual ({self.agenda_version})")
            print(f"   (Ignorando, ya tengo esta versión o una más reciente)")
        else:
            # Actualizar agenda local
            self.agenda_actual = payload
            self.agenda_version = version
            print(f"✓ Agenda actualizada (versión {version})")
            
            # Mostrar programaciones
            if programaciones:
                print("\nProgramaciones:")
                for i, prog in enumerate(programaciones, 1):
                    zona = prog.get("zona")
                    hora = prog.get("hora")
                    duracion = prog.get("duracionMinutos")
                    dias = prog.get("diasSemana", [])
                    print(f"  {i}. Zona {zona} - {hora} - {duracion}min - Días: {dias}")
        
        print(f"{'='*60}\n")
        
    def _handle_comando_zona(self, zona: str, payload: Dict[str, Any], timestamp: str):
        """Procesa un comando manual de riego en una zona"""
        print(f"\n{'='*60}")
        print(f"[{timestamp}] 💧 COMANDO MANUAL RECIBIDO")
        print(f"{'='*60}")
        
        accion = payload.get("accion", "").upper()
        duracion_seg = payload.get("duracion", 0)  # Duración en segundos según contrato
        duracion_min = duracion_seg // 60 if duracion_seg else 0
        
        print(f"Zona: {zona}")
        print(f"Acción: {accion}")
        
        if accion == "ON":
            print(f"Duración: {duracion_min} minutos ({duracion_seg} segundos)")
            print(f"\n🚿 Simulando riego en zona {zona}...")
            print(f"   └─ Electroválvula de zona {zona} ABIERTA")
            print(f"   └─ Esperando {duracion_min} minutos...")
            print(f"   └─ (En un ESP32 real, aquí activarías el relé/GPIO)")
            
            # Publicar estado (formato esperado por backend)
            status_topic = f"riego/{self.node_id}/status/zona/{zona}"
            status_payload = {
                "activa": True,
                "tiempoRestante": duracion_seg
            }
            if self.client:
                self.client.publish(status_topic, json.dumps(status_payload))
                print(f"   └─ Estado publicado en: {status_topic}")
                
        elif accion == "OFF":
            print(f"\n⏹️  Deteniendo riego en zona {zona}")
            print(f"   └─ Electroválvula de zona {zona} CERRADA")
            
            # Publicar estado (formato esperado por backend)
            status_topic = f"riego/{self.node_id}/status/zona/{zona}"
            status_payload = {
                "activa": False,
                "tiempoRestante": 0
            }
            if self.client:
                self.client.publish(status_topic, json.dumps(status_payload))
                print(f"   └─ Estado publicado en: {status_topic}")
        else:
            print(f"⚠️  Acción desconocida: {accion} (esperaba ON o OFF)")
            
        print(f"{'='*60}\n")
        
    def on_disconnect(self, client, userdata, rc, properties=None):
        """Callback cuando se desconecta del broker"""
        if rc != 0:
            print(f"\n⚠️  Desconexión inesperada del broker (código: {rc})")
        else:
            print("\n✓ Desconectado del broker")
            
    def run(self):
        """Inicia el mock ESP32"""
        print("\n" + "="*60)
        print("  MOCK ESP32 - Simulador de nodo de riego")
        print("="*60 + "\n")
        
        # Crear cliente MQTT
        self.client = mqtt.Client(
            client_id=f"mock_esp32_{self.node_id[:8]}",
            protocol=mqtt.MQTTv311
        )
        
        # Configurar callbacks
        self.client.on_connect = self.on_connect
        self.client.on_message = self.on_message
        self.client.on_disconnect = self.on_disconnect
        
        # Conectar al broker
        try:
            print(f"Conectando a {self.mqtt_host}:{self.mqtt_port}...")
            self.client.connect(self.mqtt_host, self.mqtt_port, keepalive=60)
            
            # Loop infinito para recibir mensajes
            print("Presiona Ctrl+C para detener\n")
            self.client.loop_forever()
            
        except KeyboardInterrupt:
            print("\n\n✓ Detenido por el usuario")
            self.client.disconnect()
        except ConnectionRefusedError:
            print(f"\n✗ Error: No se pudo conectar al broker en {self.mqtt_host}:{self.mqtt_port}")
            print("   ¿Está corriendo el broker MQTT (docker-compose)?")
            sys.exit(1)
        except Exception as e:
            print(f"\n✗ Error: {e}")
            sys.exit(1)


def main():
    parser = argparse.ArgumentParser(
        description="Mock ESP32 - Simulador de nodo de riego para testing"
    )
    parser.add_argument(
        "--node-id",
        required=True,
        help="UUID del nodo (ej: 550e8400-e29b-41d4-a716-446655440000)"
    )
    parser.add_argument(
        "--mqtt-host",
        default="localhost",
        help="Host del broker MQTT (default: localhost)"
    )
    parser.add_argument(
        "--mqtt-port",
        type=int,
        default=1883,
        help="Puerto del broker MQTT (default: 1883)"
    )
    
    args = parser.parse_args()
    
    # Crear y ejecutar el mock
    mock = MockESP32(
        node_id=args.node_id,
        mqtt_host=args.mqtt_host,
        mqtt_port=args.mqtt_port
    )
    mock.run()


if __name__ == "__main__":
    main()
