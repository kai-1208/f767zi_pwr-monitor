#include "pwr-monitor.h"
#include <string.h>

static UART_HandleTypeDef *s_huart = NULL;
static osSemaphoreId_t s_rxSem = NULL;

static uint8_t s_raw_rx_buf[PWR_RX_BUF_SIZE];
static char s_packet_buf[PWR_RX_BUF_SIZE];
static bool s_has_new_packet = false;

void PwrMonitor_HW_Init(UART_HandleTypeDef *huart, osSemaphoreId_t semHandle)
{
    s_huart = huart;
    s_rxSem = semHandle;

    // アイドル受信割り込み開始
    HAL_UARTEx_ReceiveToIdle_IT(s_huart, s_raw_rx_buf, PWR_RX_BUF_SIZE);
}

bool PwrMonitor_HW_GetLatestPacket(char *dest, uint16_t max_len)
{
    if (!s_has_new_packet) return false;

    strncpy(dest, s_packet_buf, max_len);
    dest[max_len - 1] = '\0';
    s_has_new_packet = false;
    return true;
}

// -------------------------------------------------------------
// HAL弱結合コールバックの実装（main.cを編集せずここで受ける）
// -------------------------------------------------------------
void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size)
{
    if (s_huart != NULL && huart == s_huart) {
        uint16_t copy_len = (Size < (PWR_RX_BUF_SIZE - 1)) ? Size : (PWR_RX_BUF_SIZE - 1);
        memcpy(s_packet_buf, s_raw_rx_buf, copy_len);
        s_packet_buf[copy_len] = '\0';
        s_has_new_packet = true;

        // セマフォを解放して待機タスクを起床
        if (s_rxSem != NULL) {
            osSemaphoreRelease(s_rxSem);
        }

        // 次の受信を再開
        HAL_UARTEx_ReceiveToIdle_IT(s_huart, s_raw_rx_buf, PWR_RX_BUF_SIZE);
    }
}

void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
    if (s_huart != NULL && huart == s_huart) {
        // エラー復旧して再開
        HAL_UARTEx_ReceiveToIdle_IT(s_huart, s_raw_rx_buf, PWR_RX_BUF_SIZE);
    }
}   