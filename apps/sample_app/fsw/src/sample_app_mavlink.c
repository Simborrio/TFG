/************************************************************************
 * Tarea hija de SAMPLE_APP: lee MAVLink del F405 (UART2/TX2/RX2 -> Pi
 * /dev/serial0) y mantiene actualizado SAMPLE_APP_Data.SensorCache.
 *
 * Corre en segundo plano, independiente del pipe de comandos. El comando
 * GET_SENSORS_CC (sample_app_cmds.c) solo lee el cache bajo mutex, nunca
 * toca el puerto serie directamente.
 ************************************************************************/

#include "sample_app.h"
#include "sample_app_eventids.h"

#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

#include "common/mavlink.h"

#define SAMPLE_APP_MAVLINK_DEVICE   "/dev/serial0"
#define SAMPLE_APP_MAVLINK_BAUD     B115200
#define SAMPLE_APP_MAVLINK_RETRY_MS 3000

/*
** Abre y configura el puerto serie en modo crudo a 115200 8N1.
** Mismo patron que apps/sbn/modules/protocol/serial (termios directo,
** sin abstraccion OSAL, ya que este build es especifico de Linux nativo).
*/
static bool SAMPLE_APP_OpenMavlinkPort(int *FdOut)
{
    int fd = open(SAMPLE_APP_MAVLINK_DEVICE, O_RDWR | O_NOCTTY);
    if (fd < 0)
    {
        CFE_EVS_SendEvent(SAMPLE_APP_MAVLINK_ERR_EID,
                          CFE_EVS_EventType_ERROR,
                          "SAMPLE_APP: no se pudo abrir %s (errno=%d)",
                          SAMPLE_APP_MAVLINK_DEVICE,
                          errno);
        return false;
    }

    struct termios tty;
    memset(&tty, 0, sizeof(tty));

    if (tcgetattr(fd, &tty) != 0)
    {
        CFE_EVS_SendEvent(SAMPLE_APP_MAVLINK_ERR_EID,
                          CFE_EVS_EventType_ERROR,
                          "SAMPLE_APP: tcgetattr fallo en %s (errno=%d)",
                          SAMPLE_APP_MAVLINK_DEVICE,
                          errno);
        close(fd);
        return false;
    }

    /* Equivalente portable a cfsetspeed()+cfmakeraw() (extensiones BSD que
     * no estan expuestas bajo los feature-test macros de este build) */
    cfsetispeed(&tty, SAMPLE_APP_MAVLINK_BAUD);
    cfsetospeed(&tty, SAMPLE_APP_MAVLINK_BAUD);

    tty.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL | IXON);
    tty.c_oflag &= ~OPOST;
    tty.c_lflag &= ~(ECHO | ECHONL | ICANON | ISIG | IEXTEN);
    tty.c_cflag &= ~(CSIZE | PARENB);
    tty.c_cflag |= CS8;
    tty.c_cc[VMIN]  = 1;
    tty.c_cc[VTIME] = 0;

    if (tcsetattr(fd, TCSANOW, &tty) != 0)
    {
        CFE_EVS_SendEvent(SAMPLE_APP_MAVLINK_ERR_EID,
                          CFE_EVS_EventType_ERROR,
                          "SAMPLE_APP: tcsetattr fallo en %s (errno=%d)",
                          SAMPLE_APP_MAVLINK_DEVICE,
                          errno);
        close(fd);
        return false;
    }

    CFE_EVS_SendEvent(SAMPLE_APP_MAVLINK_INF_EID,
                      CFE_EVS_EventType_INFORMATION,
                      "SAMPLE_APP: puerto MAVLink %s abierto (fd=%d)",
                      SAMPLE_APP_MAVLINK_DEVICE,
                      fd);

    *FdOut = fd;
    return true;
}

/*
** Vuelca los campos relevantes de un mensaje MAVLink decodificado al
** cache compartido, bajo mutex. Mensajes no reconocidos se ignoran.
*/
static void SAMPLE_APP_UpdateSensorCache(const mavlink_message_t *Msg)
{
    OS_MutSemTake(SAMPLE_APP_Data.SensorCacheMutex);

    switch (Msg->msgid)
    {
        case MAVLINK_MSG_ID_HEARTBEAT:
        {
            SAMPLE_APP_Data.SensorCache.HeartbeatSeen = true;
            break;
        }

        case MAVLINK_MSG_ID_ATTITUDE:
        {
            mavlink_attitude_t att;
            mavlink_msg_attitude_decode(Msg, &att);
            SAMPLE_APP_Data.SensorCache.LastAttitudeMs = att.time_boot_ms;
            SAMPLE_APP_Data.SensorCache.Roll           = att.roll;
            SAMPLE_APP_Data.SensorCache.Pitch          = att.pitch;
            SAMPLE_APP_Data.SensorCache.Yaw            = att.yaw;
            break;
        }

        case MAVLINK_MSG_ID_SYS_STATUS:
        {
            mavlink_sys_status_t sys;
            mavlink_msg_sys_status_decode(Msg, &sys);
            SAMPLE_APP_Data.SensorCache.BatteryVoltageMv = sys.voltage_battery;
            SAMPLE_APP_Data.SensorCache.BatteryCurrentCa = sys.current_battery;
            SAMPLE_APP_Data.SensorCache.BatteryRemaining = sys.battery_remaining;
            break;
        }

        case MAVLINK_MSG_ID_GPS_RAW_INT:
        {
            mavlink_gps_raw_int_t gps;
            mavlink_msg_gps_raw_int_decode(Msg, &gps);
            SAMPLE_APP_Data.SensorCache.Lat                = gps.lat;
            SAMPLE_APP_Data.SensorCache.Lon                = gps.lon;
            SAMPLE_APP_Data.SensorCache.AltMsl             = gps.alt;
            SAMPLE_APP_Data.SensorCache.GpsFixType         = gps.fix_type;
            SAMPLE_APP_Data.SensorCache.SatellitesVisible  = gps.satellites_visible;
            break;
        }

        case MAVLINK_MSG_ID_VFR_HUD:
        {
            mavlink_vfr_hud_t vfr;
            mavlink_msg_vfr_hud_decode(Msg, &vfr);
            SAMPLE_APP_Data.SensorCache.Groundspeed = vfr.groundspeed;
            SAMPLE_APP_Data.SensorCache.Airspeed    = vfr.airspeed;
            SAMPLE_APP_Data.SensorCache.Heading     = vfr.heading;
            break;
        }

        default:
            break;
    }

    OS_MutSemGive(SAMPLE_APP_Data.SensorCacheMutex);
}

/*
** Punto de entrada de la tarea hija. Bucle infinito: lee byte a byte,
** alimenta el parser MAVLink, y reconecta el puerto si se cae (mismo
** patron de reconexion que se aplico en los bridges LoRa Python).
*/
void SAMPLE_APP_MavlinkTask(void)
{
    int                fd = -1;
    uint8              rxbyte;
    ssize_t            n;
    mavlink_message_t  msg;
    mavlink_status_t   parse_status;

    while (!SAMPLE_APP_OpenMavlinkPort(&fd))
    {
        OS_TaskDelay(SAMPLE_APP_MAVLINK_RETRY_MS);
    }

    while (true)
    {
        n = read(fd, &rxbyte, 1);

        if (n == 1)
        {
            if (mavlink_parse_char(MAVLINK_COMM_0, rxbyte, &msg, &parse_status))
            {
                SAMPLE_APP_UpdateSensorCache(&msg);
            }
        }
        else if (n < 0 && errno != EAGAIN && errno != EINTR)
        {
            CFE_EVS_SendEvent(SAMPLE_APP_MAVLINK_ERR_EID,
                              CFE_EVS_EventType_ERROR,
                              "SAMPLE_APP: error leyendo %s (errno=%d), reconectando",
                              SAMPLE_APP_MAVLINK_DEVICE,
                              errno);
            close(fd);
            while (!SAMPLE_APP_OpenMavlinkPort(&fd))
            {
                OS_TaskDelay(SAMPLE_APP_MAVLINK_RETRY_MS);
            }
        }
        else if (n == 0)
        {
            /* EOF - dispositivo desaparecio, reconectar */
            close(fd);
            while (!SAMPLE_APP_OpenMavlinkPort(&fd))
            {
                OS_TaskDelay(SAMPLE_APP_MAVLINK_RETRY_MS);
            }
        }
    }
}
