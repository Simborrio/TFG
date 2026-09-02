#!/usr/bin/env python3
import socket
import threading
import time
import serial
from sx126x import sx126x  # mismo driver LoRa

# --- CONFIGURACIÓN ---
UDP_IP = "127.0.0.1"

# Puerto donde cFS (app CI_LAB) espera recibir comandos
CFS_CI_PORT = 1234

# Puerto donde cFS (app TO_LAB) envía la telemetría.
# TO_LAB_OUTPUT_ENABLE_CC solo fija la IP destino: el puerto real es fijo,
# calculado como TO_LAB_MISSION_TLM_PORT (2234, ver sample_defs/eds/config.xml)
# + CFE_PSP_GetProcessorId() - 1 = 2234 para cpu1. Confirmado con
# `ss -uln` mientras corría core-cpu1.
CFS_TO_PORT = 2234

# Dirección de radio de este nodo (el satélite)
SAT_ADDR = 0x0002
SAT_FREQ = 868

# Dirección de radio del nodo de tierra (destino de la telemetría)
GS_ADDR = 0x0001
GS_FREQ = 868

BASE_FREQ = 850  # offset de frecuencia, igual que en el bridge de tierra

# El módulo LoRa antepone 3 bytes de "reporte de recepción" (addr_origen_hi,
# addr_origen_lo, canal) delante del payload real. Confirmado empíricamente:
# CI_LAB reportaba paquetes de 11 bytes para un SAMPLE_APP_PingCmd_t de 8
# bytes (CFE_MSG_CommandHeader_t = 6 primario + 2 secundario) -> 3 bytes de
# cabecera que hay que descartar antes de reenviar a CI_LAB.
LORA_RX_HEADER_LEN = 3

# --- FILTRO DE TELEMETRÍA ---
# Solo estos MsgIds se reenvían por LoRa hacia tierra. Evita saturar el
# enlace de radio con HK y otro ruido de apps que no usas (LC, CF, HS,
# MM, SC, DS, etc.). Añade aquí los MsgIds que te interesen.
ALLOWED_MIDS = {
    0x088F,  # SAMPLE_APP_DATA_TLM_MID
    0x0893,  # SAMPLE_APP_SENSORS_TLM_MID
    # 0x0882,  # SAMPLE_APP_HK_TLM_MID (descomenta si también quieres el HK)
}

LORA_SERIAL_PORT = "/dev/ttyUSB0"


def open_lora():
    return sx126x(
        serial_num=LORA_SERIAL_PORT,
        freq=SAT_FREQ,
        addr=SAT_ADDR,
        power=22,
        rssi=False
    )


# --- CONFIGURA LORA ---
lora = open_lora()

# Protege el acceso concurrente al puerto serie del LoRa entre los dos hilos
lora_lock = threading.Lock()


def reconnect_lora():
    """Reabre el puerto serie y reconfigura el módulo tras un error de I/O
    (p.ej. el adaptador USB se desconectó momentáneamente)."""
    global lora
    while True:
        try:
            print("  Puerto LoRa caído, reconectando...")
            with lora_lock:
                try:
                    lora.ser.close()
                except Exception:
                    pass
                lora = open_lora()
            print("  Módulo LoRa reconectado.")
            return
        except Exception as e:
            print(f"  Fallo al reconectar LoRa ({e}), reintento en 3s...")
            time.sleep(3)

# --- SOCKETS UDP ---
# Socket para enviar comandos recibidos por LoRa hacia CI (cFS)
sock_to_ci = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

# Socket para escuchar la telemetría que envía TO (cFS)
sock_from_to = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock_from_to.bind((UDP_IP, CFS_TO_PORT))

print("LoRa bridge conectado a cFS (satélite)")
print(f"Reenviando comandos LoRa -> CI en {UDP_IP}:{CFS_CI_PORT}")
print(f"Escuchando telemetría de TO en {UDP_IP}:{CFS_TO_PORT} -> LoRa hacia GS")
print(f"MsgIds permitidos hacia LoRa: {[hex(m) for m in ALLOWED_MIDS]}")


# --- HILO 1: LoRa (comandos desde tierra) → cFS CI ---
def lora_to_cfs():
    while True:
        try:
            data = None
            with lora_lock:
                if lora.ser.inWaiting() > 0:
                    time.sleep(0.3)
                    data = lora.ser.read(lora.ser.inWaiting())

            if data:
                payload = data[LORA_RX_HEADER_LEN:]
                print(f"  Comando recibido por LoRa ({len(data)} bytes, "
                      f"payload {len(payload)}) -> CI")
                sock_to_ci.sendto(payload, (UDP_IP, CFS_CI_PORT))
            else:
                time.sleep(0.05)  # evita busy-wait al 100% de CPU
        except (OSError, serial.SerialException) as e:
            print(f"  Error de I/O leyendo LoRa ({e})")
            reconnect_lora()


# --- HILO 2: cFS TO (telemetría) → LoRa hacia tierra ---
def cfs_to_lora():
    while True:
        data, addr = sock_from_to.recvfrom(4096)

        if len(data) < 2:
            continue

        # Los MsgIds de cFS van en los primeros 2 bytes, big-endian
        msgid = (data[0] << 8) | data[1]

        if msgid not in ALLOWED_MIDS:
            continue  # descarta HK y demás telemetría no filtrada

        print(f"  Telemetría 0x{msgid:04X} ({len(data)} bytes) -> LoRa")

        offset_freq = GS_FREQ - BASE_FREQ
        pkt = (
            bytes([GS_ADDR >> 8]) +
            bytes([GS_ADDR & 0xff]) +
            bytes([offset_freq]) +
            bytes([lora.addr >> 8]) +
            bytes([lora.addr & 0xff]) +
            bytes([lora.offset_freq]) +
            data
        )

        try:
            with lora_lock:
                lora.send(pkt)
        except (OSError, serial.SerialException) as e:
            print(f"  Error de I/O enviando por LoRa ({e})")
            reconnect_lora()


threading.Thread(target=lora_to_cfs, daemon=True).start()
threading.Thread(target=cfs_to_lora, daemon=True).start()

try:
    while True:
        time.sleep(1)
except KeyboardInterrupt:
    print("Bridge detenido.")
