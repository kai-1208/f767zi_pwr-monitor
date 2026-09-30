#include "pwr-monitor.h"
#include "usart.h"
#include <cstdio>
#include <cstring>

class PowerMonitorController {
public:
    PwrStatus_t status{0, 0, 0, false};
    uint32_t last_rx_tick{0};
    const uint32_t TIMEOUT_MS{500};

    bool processPacket(const char* packet) {
        int p4, p5, p6;
        if (sscanf(packet, "PWR:%d,%d,%d", &p4, &p5, &p6) == 3) {
            status.pa4 = static_cast<uint8_t>(p4);
            status.pa5 = static_cast<uint8_t>(p5);
            status.pa6 = static_cast<uint8_t>(p6);
            status.is_online = true;
            last_rx_tick = osKernelGetTickCount();
            return true;
        }
        return false;
    }

    void checkTimeout() {
        if (status.is_online) {
            uint32_t current_tick = osKernelGetTickCount();
            if ((current_tick - last_rx_tick) > TIMEOUT_MS) {
                status.is_online = false;
            }
        }
    }
};

static PowerMonitorController s_controller;

// 外部参照用API
extern "C" PwrStatus_t PwrMonitor_GetStatus(void) {
    return s_controller.status;
}

extern "C" void PwrMonitor_Task(void *argument)
{
    PwrMonitor_HW_Init(&huart6, pwrRxSemHandle);

    char packet[PWR_RX_BUF_SIZE];
    uint32_t next_report_tick = osKernelGetTickCount();

    for (;;) {
        // 200msタイムアウト付きでセマフォ待機
        osStatus_t sem_status = osSemaphoreAcquire(pwrRxSemHandle, 200);

        if (sem_status == osOK) {
            if (PwrMonitor_HW_GetLatestPacket(packet, sizeof(packet))) {
                s_controller.processPacket(packet);
            }
        }

        // 途絶チェック
        s_controller.checkTimeout();

        const uint32_t current_tick = osKernelGetTickCount();
        if (static_cast<int32_t>(current_tick - next_report_tick) >= 0) {
            const PwrStatus_t status = PwrMonitor_GetStatus();
            char line[48];
            const int length = std::snprintf(
                line,
                sizeof(line),
                "PA4=%s PA5=%s PA6=%s\r\n",
                status.pa4 != 0U ? "true" : "false",
                status.pa5 != 0U ? "true" : "false",
                status.pa6 != 0U ? "true" : "false");

            if (length < 0 || static_cast<size_t>(length) >= sizeof(line)) {
                Error_Handler();
            }
            if (HAL_UART_Transmit(
                    &huart3,
                    reinterpret_cast<uint8_t *>(line),
                    static_cast<uint16_t>(length),
                    HAL_MAX_DELAY) != HAL_OK) {
                Error_Handler();
            }
            next_report_tick = current_tick + 500U;
        }
    }
}