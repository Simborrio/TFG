#!/usr/bin/env python3
import socket
import threading
import time
from sx126x import sx126x  # tu driver LoRa

# --- CONFIGURACIÓN ---
UDP_IP = "127.0.0.1"
UDP_PORT_CMD = 1234   # GroundSystem manda comandos a este puerto
UDP_PORT_TLM = 1235   # GroundSystem recibe telemetría de este puerto
DEST_ADDR = 0x0002
DEST_FREQ = 868
BASE_FREQ = 850  # porque el driver calcula offset como (freq - 850)

# El módulo LoRa antepone 3 bytes de "reporte de recepción" (addr_origen_hi,
# addr_origen_lo, canal) delante del payload real - confirmado en el lado FS
# (bridge/lora_bridge_fs.py) al ver CI_LAB recibir 11 bytes para un comando
# de 8. Aplica igual aquí para la telemetría que llega desde el satélite.
LORA_RX_HEADER_LEN = 3
# --- CONFIGURA LORA ---
lora = sx126x(
    serial_num="/dev/ttyS0",  # puerto UART del HAT
    freq=868,                 # frecuencia MHz
    addr=0x0001,              # dirección del nodo local
    power=22,                 # potencia
    rssi=False
)

# --- SOCKETS UDP ---
sock_cmd = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock_cmd.bind((UDP_IP, UDP_PORT_CMD))  # donde GS manda comandos

sock_tlm = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)  # donde GS recibirá telemetría

print("LoRa bridge conectado al GroundSystem")
print(f"Escuchando comandos UDP en {UDP_IP}:{UDP_PORT_CMD}")
print(f"Enviando telemetría UDP a {UDP_IP}:{UDP_PORT_TLM}")

# --- HILO 1: GroundSystem → LoRa ---
def gs_to_lora():
    while True:
        data, addr = sock_cmd.recvfrom(4096)
        print(f"  Comando desde GroundSystem ({len(data)} bytes)")
        
        # Arma el paquete con cabecera
        offset_freq = DEST_FREQ - BASE_FREQ
        pkt = (
            bytes([DEST_ADDR >> 8]) +
            bytes([DEST_ADDR & 0xff]) +
            bytes([offset_freq]) +
            bytes([lora.addr >> 8]) +
            bytes([lora.addr & 0xff]) +
            bytes([lora.offset_freq]) +
            data
        )
        
        lora.send(pkt)

# --- HILO 2: LoRa → GroundSystem ---
def lora_to_gs():
    while True:
        if lora.ser.inWaiting() > 0:
            time.sleep(0.3)
            msg = lora.ser.read(lora.ser.inWaiting())
            if msg:
                payload = msg[LORA_RX_HEADER_LEN:]
                print(f"  Telemetría recibida por LoRa ({len(msg)} bytes, "
                      f"payload {len(payload)} bytes)")
                sock_tlm.sendto(payload, (UDP_IP, UDP_PORT_TLM))

threading.Thread(target=gs_to_lora, daemon=True).start()
threading.Thread(target=lora_to_gs, daemon=True).start()

try:
    while True:
        time.sleep(1)
except KeyboardInterrupt:
    print("Bridge detenido.")
