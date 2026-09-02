#!/usr/bin/env python3
# Diagnóstico: escucha crudo en el puerto serie del LoRa, sin pasar nada a cFS.
# Sirve para confirmar si llega ALGO por el aire cuando tierra transmite.
import time
from sx126x import sx126x

SAT_ADDR = 0x0002
SAT_FREQ = 868

lora = sx126x(serial_num="/dev/ttyUSB0", freq=SAT_FREQ, addr=SAT_ADDR, power=22, rssi=False)
print(f"Escuchando crudo en /dev/ttyUSB0 (addr 0x{SAT_ADDR:04X}, freq {SAT_FREQ}). Ctrl+C para salir.")

try:
    while True:
        if lora.ser.inWaiting() > 0:
            time.sleep(0.3)
            data = lora.ser.read(lora.ser.inWaiting())
            print(f"[{time.strftime('%H:%M:%S')}] {len(data)} bytes: {data.hex()}")
        else:
            time.sleep(0.05)
except KeyboardInterrupt:
    print("\nDetenido.")
