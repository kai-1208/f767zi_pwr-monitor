#ifndef PWR_MONITOR_H
#define PWR_MONITOR_H

#include "main.h"
#include "cmsis_os2.h"
#include <stdbool.h>

#define PWR_RX_BUF_SIZE 64

#ifdef __cplusplus
extern "C" {
#endif

// 低レベル初期化（UARTハンドルと同期セマフォをバインド）
void PwrMonitor_HW_Init(UART_HandleTypeDef *huart, osSemaphoreId_t semHandle);
extern osSemaphoreId_t pwrRxSemHandle;

// 最新パケットの取得（高レベル層向け）
bool PwrMonitor_HW_GetLatestPacket(char *dest, uint16_t max_len);

// 外部（NeoPixel等）から電源状態を取得するための構造体とAPI
typedef struct {
    uint8_t pa4;
    uint8_t pa5;
    uint8_t pa6;
    bool is_online; // 通信が正常に継続しているか
} PwrStatus_t;

PwrStatus_t PwrMonitor_GetStatus(void);
void PwrMonitor_Task(void *argument);

#ifdef __cplusplus
}
#endif

#endif /* PWR_MONITOR_H */