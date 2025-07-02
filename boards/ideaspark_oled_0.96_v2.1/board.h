#ifndef BOARD_H
#define BOARD_H

// Pin definitions for Ideaspark Oled 0.96 v2.1
#define OLED_SDA 12   // D6 on NodeMCU - Alternative I2C pins for this board
#define OLED_SCL 14   // D5 on NodeMCU - Alternative I2C pins for this board
#define OLED_RST 16   // D0 on NodeMCU (often unused)

#define UART_PORT_NUM      UART_NUM_0
#define UART_BAUD_RATE     115200
#define UART_BUF_SIZE      512
#define UART_TX_PIN        1    // TX on NodeMCU
#define LED_PIN            2    // D4 on NodeMCU (built-in LED)
#define UART_RX_PIN        3    // RX on NodeMCU
#define FLASH_BUTTON_PIN   0    // FLASH button on NodeMCU

#endif // BOARD_H
