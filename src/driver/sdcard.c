#include "../lib/stdio.h"
#include "../lib/memory.h"
#include "sdcard.h"

typedef unsigned int uint;
typedef unsigned short ushort;
typedef unsigned char uchar;
typedef unsigned short wchar;

typedef unsigned char uint8;
typedef unsigned short uint16;
typedef unsigned int uint32;
typedef unsigned long uint64;

typedef unsigned long uintptr_t;
typedef uint64 pde_t;

#define NULL ((void *)0)

#define readb(addr) (*(volatile uint8 *)(addr))
#define readw(addr) (*(volatile uint16 *)(addr))
#define readd(addr) (*(volatile uint32 *)(addr))
#define readq(addr) (*(volatile uint64 *)(addr))

#define writeb(v, addr)                      \
    {                                        \
        (*(volatile uint8 *)(addr)) = (v); \
    }
#define writew(v, addr)                       \
    {                                         \
        (*(volatile uint16 *)(addr)) = (v); \
    }
#define writed(v, addr)                       \
    {                                         \
        (*(volatile uint32 *)(addr)) = (v); \
    }
#define writeq(v, addr)                       \
    {                                         \
        (*(volatile uint64 *)(addr)) = (v); \
    }


typedef enum _gpio_drive_mode {
    GPIO_DM_INPUT,
    GPIO_DM_INPUT_PULL_DOWN,
    GPIO_DM_INPUT_PULL_UP,
    GPIO_DM_OUTPUT,
} gpio_drive_mode_t;

typedef enum _gpio_pin_edge {
    GPIO_PE_NONE,
    GPIO_PE_FALLING,
    GPIO_PE_RISING,
    GPIO_PE_BOTH,
    GPIO_PE_LOW,
    GPIO_PE_HIGH = 8,
} gpio_pin_edge_t;

typedef enum _gpio_pin_value {
    GPIO_PV_LOW,
    GPIO_PV_HIGH
} gpio_pin_value_t;


// func
void set_bit(volatile uint32 *bits, uint32 mask, uint32 value) {
    printf("0x%x\n", bits);
    uint32 org = (*bits) & ~mask;
    *bits = org | (value & mask);
}

void set_bit_offset(volatile uint32 *bits, uint32 mask, uint64 offset, uint32 value) {
    set_bit(bits, mask << offset, value << offset);
}

void set_gpio_bit(volatile uint32 *bits, uint64 offset, uint32 value) {

    set_bit_offset(bits, 1, offset, value);
}

uint32 get_bit(volatile uint32 *bits, uint32 mask, uint64 offset) {
    return ((*bits) & (mask << offset)) >> offset;
}

uint32 get_gpio_bit(volatile uint32 *bits, uint64 offset) {
    return get_bit(bits, 1, offset);
}

/**
 * @brief       GPIO bits object
 */
typedef struct _gpiohs_bits {
    uint32 b0: 1;
    uint32 b1: 1;
    uint32 b2: 1;
    uint32 b3: 1;
    uint32 b4: 1;
    uint32 b5: 1;
    uint32 b6: 1;
    uint32 b7: 1;
    uint32 b8: 1;
    uint32 b9: 1;
    uint32 b10: 1;
    uint32 b11: 1;
    uint32 b12: 1;
    uint32 b13: 1;
    uint32 b14: 1;
    uint32 b15: 1;
    uint32 b16: 1;
    uint32 b17: 1;
    uint32 b18: 1;
    uint32 b19: 1;
    uint32 b20: 1;
    uint32 b21: 1;
    uint32 b22: 1;
    uint32 b23: 1;
    uint32 b24: 1;
    uint32 b25: 1;
    uint32 b26: 1;
    uint32 b27: 1;
    uint32 b28: 1;
    uint32 b29: 1;
    uint32 b30: 1;
    uint32 b31: 1;
} __attribute__((packed, aligned(4))) gpiohs_bits_t;
/**
 * @brief       GPIO bits multi access union
 */
typedef union _gpiohs_u32 {
    /* 32x1 bit mode */
    uint32 u32[1];
    /* 16x2 bit mode */
    uint16 u16[2];
    /* 8x4 bit mode */
    uint8 u8[4];
    /* 1 bit mode */
    gpiohs_bits_t bits;
} __attribute__((packed, aligned(4))) gpiohs_u32_t;
typedef struct _gpiohs {
    /* Address offset 0x00, Input Values */
    gpiohs_u32_t input_val;
    /* Address offset 0x04, Input enable */
    gpiohs_u32_t input_en;
    /* Address offset 0x08, Output enable */
    gpiohs_u32_t output_en;
    /* Address offset 0x0c, Onput Values */
    gpiohs_u32_t output_val;
    /* Address offset 0x10, Internal Pull-Ups enable */
    gpiohs_u32_t pullup_en;
    /* Address offset 0x14, Drive Strength */
    gpiohs_u32_t drive;
    /* Address offset 0x18, Rise interrupt enable */
    gpiohs_u32_t rise_ie;
    /* Address offset 0x1c, Rise interrupt pending */
    gpiohs_u32_t rise_ip;
    /* Address offset 0x20, Fall interrupt enable */
    gpiohs_u32_t fall_ie;
    /* Address offset 0x24, Fall interrupt pending */
    gpiohs_u32_t fall_ip;
    /* Address offset 0x28, High interrupt enable */
    gpiohs_u32_t high_ie;
    /* Address offset 0x2c, High interrupt pending */
    gpiohs_u32_t high_ip;
    /* Address offset 0x30, Low interrupt enable */
    gpiohs_u32_t low_ie;
    /* Address offset 0x34, Low interrupt pending */
    gpiohs_u32_t low_ip;
    /* Address offset 0x38, HW I/O Function enable */
    gpiohs_u32_t iof_en;
    /* Address offset 0x3c, HW I/O Function select */
    gpiohs_u32_t iof_sel;
    /* Address offset 0x40, Output XOR (invert) */
    gpiohs_u32_t output_xor;
} __attribute__((packed, aligned(4))) gpiohs_t;
#define GPIOHS_BASE_ADDR    (0x38001000U)
volatile gpiohs_t *const gpiohs = (volatile gpiohs_t *) GPIOHS_BASE_ADDR;

void gpiohs_set_pin(uint8 pin, gpio_pin_value_t value) {
    // configASSERT(pin < GPIOHS_MAX_PINNO);
    set_gpio_bit(gpiohs->output_val.u32, pin, value);
}

void SD_CS_HIGH(void) {
    gpiohs_set_pin(7, GPIO_PV_HIGH);
}

void SD_CS_LOW(void) {
    gpiohs_set_pin(7, GPIO_PV_LOW);
}

void SD_HIGH_SPEED_ENABLE(void) {
    // spi_set_clk_rate(SPI_DEVICE_0, 10000000);
}

typedef enum _fpioa_function {
    FUNC_JTAG_TCLK = 0,  /*!< JTAG Test Clock */
    FUNC_JTAG_TDI = 1,  /*!< JTAG Test Data In */
    FUNC_JTAG_TMS = 2,  /*!< JTAG Test Mode Select */
    FUNC_JTAG_TDO = 3,  /*!< JTAG Test Data Out */
    FUNC_SPI0_D0 = 4,  /*!< SPI0 Data 0 */
    FUNC_SPI0_D1 = 5,  /*!< SPI0 Data 1 */
    FUNC_SPI0_D2 = 6,  /*!< SPI0 Data 2 */
    FUNC_SPI0_D3 = 7,  /*!< SPI0 Data 3 */
    FUNC_SPI0_D4 = 8,  /*!< SPI0 Data 4 */
    FUNC_SPI0_D5 = 9,  /*!< SPI0 Data 5 */
    FUNC_SPI0_D6 = 10, /*!< SPI0 Data 6 */
    FUNC_SPI0_D7 = 11, /*!< SPI0 Data 7 */
    FUNC_SPI0_SS0 = 12, /*!< SPI0 Chip Select 0 */
    FUNC_SPI0_SS1 = 13, /*!< SPI0 Chip Select 1 */
    FUNC_SPI0_SS2 = 14, /*!< SPI0 Chip Select 2 */
    FUNC_SPI0_SS3 = 15, /*!< SPI0 Chip Select 3 */
    FUNC_SPI0_ARB = 16, /*!< SPI0 Arbitration */
    FUNC_SPI0_SCLK = 17, /*!< SPI0 Serial Clock */
    FUNC_UARTHS_RX = 18, /*!< UART High speed Receiver */
    FUNC_UARTHS_TX = 19, /*!< UART High speed Transmitter */
    FUNC_RESV6 = 20, /*!< Reserved function */
    FUNC_RESV7 = 21, /*!< Reserved function */
    FUNC_CLK_SPI1 = 22, /*!< Clock SPI1 */
    FUNC_CLK_I2C1 = 23, /*!< Clock I2C1 */
    FUNC_GPIOHS0 = 24, /*!< GPIO High speed 0 */
    FUNC_GPIOHS1 = 25, /*!< GPIO High speed 1 */
    FUNC_GPIOHS2 = 26, /*!< GPIO High speed 2 */
    FUNC_GPIOHS3 = 27, /*!< GPIO High speed 3 */
    FUNC_GPIOHS4 = 28, /*!< GPIO High speed 4 */
    FUNC_GPIOHS5 = 29, /*!< GPIO High speed 5 */
    FUNC_GPIOHS6 = 30, /*!< GPIO High speed 6 */
    FUNC_GPIOHS7 = 31, /*!< GPIO High speed 7 */
    FUNC_GPIOHS8 = 32, /*!< GPIO High speed 8 */
    FUNC_GPIOHS9 = 33, /*!< GPIO High speed 9 */
    FUNC_GPIOHS10 = 34, /*!< GPIO High speed 10 */
    FUNC_GPIOHS11 = 35, /*!< GPIO High speed 11 */
    FUNC_GPIOHS12 = 36, /*!< GPIO High speed 12 */
    FUNC_GPIOHS13 = 37, /*!< GPIO High speed 13 */
    FUNC_GPIOHS14 = 38, /*!< GPIO High speed 14 */
    FUNC_GPIOHS15 = 39, /*!< GPIO High speed 15 */
    FUNC_GPIOHS16 = 40, /*!< GPIO High speed 16 */
    FUNC_GPIOHS17 = 41, /*!< GPIO High speed 17 */
    FUNC_GPIOHS18 = 42, /*!< GPIO High speed 18 */
    FUNC_GPIOHS19 = 43, /*!< GPIO High speed 19 */
    FUNC_GPIOHS20 = 44, /*!< GPIO High speed 20 */
    FUNC_GPIOHS21 = 45, /*!< GPIO High speed 21 */
    FUNC_GPIOHS22 = 46, /*!< GPIO High speed 22 */
    FUNC_GPIOHS23 = 47, /*!< GPIO High speed 23 */
    FUNC_GPIOHS24 = 48, /*!< GPIO High speed 24 */
    FUNC_GPIOHS25 = 49, /*!< GPIO High speed 25 */
    FUNC_GPIOHS26 = 50, /*!< GPIO High speed 26 */
    FUNC_GPIOHS27 = 51, /*!< GPIO High speed 27 */
    FUNC_GPIOHS28 = 52, /*!< GPIO High speed 28 */
    FUNC_GPIOHS29 = 53, /*!< GPIO High speed 29 */
    FUNC_GPIOHS30 = 54, /*!< GPIO High speed 30 */
    FUNC_GPIOHS31 = 55, /*!< GPIO High speed 31 */
    FUNC_GPIO0 = 56, /*!< GPIO pin 0 */
    FUNC_GPIO1 = 57, /*!< GPIO pin 1 */
    FUNC_GPIO2 = 58, /*!< GPIO pin 2 */
    FUNC_GPIO3 = 59, /*!< GPIO pin 3 */
    FUNC_GPIO4 = 60, /*!< GPIO pin 4 */
    FUNC_GPIO5 = 61, /*!< GPIO pin 5 */
    FUNC_GPIO6 = 62, /*!< GPIO pin 6 */
    FUNC_GPIO7 = 63, /*!< GPIO pin 7 */
    FUNC_UART1_RX = 64, /*!< UART1 Receiver */
    FUNC_UART1_TX = 65, /*!< UART1 Transmitter */
    FUNC_UART2_RX = 66, /*!< UART2 Receiver */
    FUNC_UART2_TX = 67, /*!< UART2 Transmitter */
    FUNC_UART3_RX = 68, /*!< UART3 Receiver */
    FUNC_UART3_TX = 69, /*!< UART3 Transmitter */
    FUNC_SPI1_D0 = 70, /*!< SPI1 Data 0 */
    FUNC_SPI1_D1 = 71, /*!< SPI1 Data 1 */
    FUNC_SPI1_D2 = 72, /*!< SPI1 Data 2 */
    FUNC_SPI1_D3 = 73, /*!< SPI1 Data 3 */
    FUNC_SPI1_D4 = 74, /*!< SPI1 Data 4 */
    FUNC_SPI1_D5 = 75, /*!< SPI1 Data 5 */
    FUNC_SPI1_D6 = 76, /*!< SPI1 Data 6 */
    FUNC_SPI1_D7 = 77, /*!< SPI1 Data 7 */
    FUNC_SPI1_SS0 = 78, /*!< SPI1 Chip Select 0 */
    FUNC_SPI1_SS1 = 79, /*!< SPI1 Chip Select 1 */
    FUNC_SPI1_SS2 = 80, /*!< SPI1 Chip Select 2 */
    FUNC_SPI1_SS3 = 81, /*!< SPI1 Chip Select 3 */
    FUNC_SPI1_ARB = 82, /*!< SPI1 Arbitration */
    FUNC_SPI1_SCLK = 83, /*!< SPI1 Serial Clock */
    FUNC_SPI_SLAVE_D0 = 84, /*!< SPI Slave Data 0 */
    FUNC_SPI_SLAVE_SS = 85, /*!< SPI Slave Select */
    FUNC_SPI_SLAVE_SCLK = 86, /*!< SPI Slave Serial Clock */
    FUNC_I2S0_MCLK = 87, /*!< I2S0 Master Clock */
    FUNC_I2S0_SCLK = 88, /*!< I2S0 Serial Clock(BCLK) */
    FUNC_I2S0_WS = 89, /*!< I2S0 Word Select(LRCLK) */
    FUNC_I2S0_IN_D0 = 90, /*!< I2S0 Serial Data Input 0 */
    FUNC_I2S0_IN_D1 = 91, /*!< I2S0 Serial Data Input 1 */
    FUNC_I2S0_IN_D2 = 92, /*!< I2S0 Serial Data Input 2 */
    FUNC_I2S0_IN_D3 = 93, /*!< I2S0 Serial Data Input 3 */
    FUNC_I2S0_OUT_D0 = 94, /*!< I2S0 Serial Data Output 0 */
    FUNC_I2S0_OUT_D1 = 95, /*!< I2S0 Serial Data Output 1 */
    FUNC_I2S0_OUT_D2 = 96, /*!< I2S0 Serial Data Output 2 */
    FUNC_I2S0_OUT_D3 = 97, /*!< I2S0 Serial Data Output 3 */
    FUNC_I2S1_MCLK = 98, /*!< I2S1 Master Clock */
    FUNC_I2S1_SCLK = 99, /*!< I2S1 Serial Clock(BCLK) */
    FUNC_I2S1_WS = 100,    /*!< I2S1 Word Select(LRCLK) */
    FUNC_I2S1_IN_D0 = 101,    /*!< I2S1 Serial Data Input 0 */
    FUNC_I2S1_IN_D1 = 102,    /*!< I2S1 Serial Data Input 1 */
    FUNC_I2S1_IN_D2 = 103,    /*!< I2S1 Serial Data Input 2 */
    FUNC_I2S1_IN_D3 = 104,    /*!< I2S1 Serial Data Input 3 */
    FUNC_I2S1_OUT_D0 = 105,    /*!< I2S1 Serial Data Output 0 */
    FUNC_I2S1_OUT_D1 = 106,    /*!< I2S1 Serial Data Output 1 */
    FUNC_I2S1_OUT_D2 = 107,    /*!< I2S1 Serial Data Output 2 */
    FUNC_I2S1_OUT_D3 = 108,    /*!< I2S1 Serial Data Output 3 */
    FUNC_I2S2_MCLK = 109,    /*!< I2S2 Master Clock */
    FUNC_I2S2_SCLK = 110,    /*!< I2S2 Serial Clock(BCLK) */
    FUNC_I2S2_WS = 111,    /*!< I2S2 Word Select(LRCLK) */
    FUNC_I2S2_IN_D0 = 112,    /*!< I2S2 Serial Data Input 0 */
    FUNC_I2S2_IN_D1 = 113,    /*!< I2S2 Serial Data Input 1 */
    FUNC_I2S2_IN_D2 = 114,    /*!< I2S2 Serial Data Input 2 */
    FUNC_I2S2_IN_D3 = 115,    /*!< I2S2 Serial Data Input 3 */
    FUNC_I2S2_OUT_D0 = 116,    /*!< I2S2 Serial Data Output 0 */
    FUNC_I2S2_OUT_D1 = 117,    /*!< I2S2 Serial Data Output 1 */
    FUNC_I2S2_OUT_D2 = 118,    /*!< I2S2 Serial Data Output 2 */
    FUNC_I2S2_OUT_D3 = 119,    /*!< I2S2 Serial Data Output 3 */
    FUNC_RESV0 = 120,    /*!< Reserved function */
    FUNC_RESV1 = 121,    /*!< Reserved function */
    FUNC_RESV2 = 122,    /*!< Reserved function */
    FUNC_RESV3 = 123,    /*!< Reserved function */
    FUNC_RESV4 = 124,    /*!< Reserved function */
    FUNC_RESV5 = 125,    /*!< Reserved function */
    FUNC_I2C0_SCLK = 126,    /*!< I2C0 Serial Clock */
    FUNC_I2C0_SDA = 127,    /*!< I2C0 Serial Data */
    FUNC_I2C1_SCLK = 128,    /*!< I2C1 Serial Clock */
    FUNC_I2C1_SDA = 129,    /*!< I2C1 Serial Data */
    FUNC_I2C2_SCLK = 130,    /*!< I2C2 Serial Clock */
    FUNC_I2C2_SDA = 131,    /*!< I2C2 Serial Data */
    FUNC_CMOS_XCLK = 132,    /*!< DVP System Clock */
    FUNC_CMOS_RST = 133,    /*!< DVP System Reset */
    FUNC_CMOS_PWDN = 134,    /*!< DVP Power Down Mode */
    FUNC_CMOS_VSYNC = 135,    /*!< DVP Vertical Sync */
    FUNC_CMOS_HREF = 136,    /*!< DVP Horizontal Reference output */
    FUNC_CMOS_PCLK = 137,    /*!< Pixel Clock */
    FUNC_CMOS_D0 = 138,    /*!< Data Bit 0 */
    FUNC_CMOS_D1 = 139,    /*!< Data Bit 1 */
    FUNC_CMOS_D2 = 140,    /*!< Data Bit 2 */
    FUNC_CMOS_D3 = 141,    /*!< Data Bit 3 */
    FUNC_CMOS_D4 = 142,    /*!< Data Bit 4 */
    FUNC_CMOS_D5 = 143,    /*!< Data Bit 5 */
    FUNC_CMOS_D6 = 144,    /*!< Data Bit 6 */
    FUNC_CMOS_D7 = 145,    /*!< Data Bit 7 */
    FUNC_SCCB_SCLK = 146,    /*!< SCCB Serial Clock */
    FUNC_SCCB_SDA = 147,    /*!< SCCB Serial Data */
    FUNC_UART1_CTS = 148,    /*!< UART1 Clear To Send */
    FUNC_UART1_DSR = 149,    /*!< UART1 Data Set Ready */
    FUNC_UART1_DCD = 150,    /*!< UART1 Data Carrier Detect */
    FUNC_UART1_RI = 151,    /*!< UART1 Ring Indicator */
    FUNC_UART1_SIR_IN = 152,    /*!< UART1 Serial Infrared Input */
    FUNC_UART1_DTR = 153,    /*!< UART1 Data Terminal Ready */
    FUNC_UART1_RTS = 154,    /*!< UART1 Request To Send */
    FUNC_UART1_OUT2 = 155,    /*!< UART1 User-designated Output 2 */
    FUNC_UART1_OUT1 = 156,    /*!< UART1 User-designated Output 1 */
    FUNC_UART1_SIR_OUT = 157,    /*!< UART1 Serial Infrared Output */
    FUNC_UART1_BAUD = 158,    /*!< UART1 Transmit Clock Output */
    FUNC_UART1_RE = 159,    /*!< UART1 Receiver Output Enable */
    FUNC_UART1_DE = 160,    /*!< UART1 Driver Output Enable */
    FUNC_UART1_RS485_EN = 161,    /*!< UART1 RS485 Enable */
    FUNC_UART2_CTS = 162,    /*!< UART2 Clear To Send */
    FUNC_UART2_DSR = 163,    /*!< UART2 Data Set Ready */
    FUNC_UART2_DCD = 164,    /*!< UART2 Data Carrier Detect */
    FUNC_UART2_RI = 165,    /*!< UART2 Ring Indicator */
    FUNC_UART2_SIR_IN = 166,    /*!< UART2 Serial Infrared Input */
    FUNC_UART2_DTR = 167,    /*!< UART2 Data Terminal Ready */
    FUNC_UART2_RTS = 168,    /*!< UART2 Request To Send */
    FUNC_UART2_OUT2 = 169,    /*!< UART2 User-designated Output 2 */
    FUNC_UART2_OUT1 = 170,    /*!< UART2 User-designated Output 1 */
    FUNC_UART2_SIR_OUT = 171,    /*!< UART2 Serial Infrared Output */
    FUNC_UART2_BAUD = 172,    /*!< UART2 Transmit Clock Output */
    FUNC_UART2_RE = 173,    /*!< UART2 Receiver Output Enable */
    FUNC_UART2_DE = 174,    /*!< UART2 Driver Output Enable */
    FUNC_UART2_RS485_EN = 175,    /*!< UART2 RS485 Enable */
    FUNC_UART3_CTS = 176,    /*!< UART3 Clear To Send */
    FUNC_UART3_DSR = 177,    /*!< UART3 Data Set Ready */
    FUNC_UART3_DCD = 178,    /*!< UART3 Data Carrier Detect */
    FUNC_UART3_RI = 179,    /*!< UART3 Ring Indicator */
    FUNC_UART3_SIR_IN = 180,    /*!< UART3 Serial Infrared Input */
    FUNC_UART3_DTR = 181,    /*!< UART3 Data Terminal Ready */
    FUNC_UART3_RTS = 182,    /*!< UART3 Request To Send */
    FUNC_UART3_OUT2 = 183,    /*!< UART3 User-designated Output 2 */
    FUNC_UART3_OUT1 = 184,    /*!< UART3 User-designated Output 1 */
    FUNC_UART3_SIR_OUT = 185,    /*!< UART3 Serial Infrared Output */
    FUNC_UART3_BAUD = 186,    /*!< UART3 Transmit Clock Output */
    FUNC_UART3_RE = 187,    /*!< UART3 Receiver Output Enable */
    FUNC_UART3_DE = 188,    /*!< UART3 Driver Output Enable */
    FUNC_UART3_RS485_EN = 189,    /*!< UART3 RS485 Enable */
    FUNC_TIMER0_TOGGLE1 = 190,    /*!< TIMER0 Toggle Output 1 */
    FUNC_TIMER0_TOGGLE2 = 191,    /*!< TIMER0 Toggle Output 2 */
    FUNC_TIMER0_TOGGLE3 = 192,    /*!< TIMER0 Toggle Output 3 */
    FUNC_TIMER0_TOGGLE4 = 193,    /*!< TIMER0 Toggle Output 4 */
    FUNC_TIMER1_TOGGLE1 = 194,    /*!< TIMER1 Toggle Output 1 */
    FUNC_TIMER1_TOGGLE2 = 195,    /*!< TIMER1 Toggle Output 2 */
    FUNC_TIMER1_TOGGLE3 = 196,    /*!< TIMER1 Toggle Output 3 */
    FUNC_TIMER1_TOGGLE4 = 197,    /*!< TIMER1 Toggle Output 4 */
    FUNC_TIMER2_TOGGLE1 = 198,    /*!< TIMER2 Toggle Output 1 */
    FUNC_TIMER2_TOGGLE2 = 199,    /*!< TIMER2 Toggle Output 2 */
    FUNC_TIMER2_TOGGLE3 = 200,    /*!< TIMER2 Toggle Output 3 */
    FUNC_TIMER2_TOGGLE4 = 201,    /*!< TIMER2 Toggle Output 4 */
    FUNC_CLK_SPI2 = 202,    /*!< Clock SPI2 */
    FUNC_CLK_I2C2 = 203,    /*!< Clock I2C2 */
    FUNC_INTERNAL0 = 204,    /*!< Internal function signal 0 */
    FUNC_INTERNAL1 = 205,    /*!< Internal function signal 1 */
    FUNC_INTERNAL2 = 206,    /*!< Internal function signal 2 */
    FUNC_INTERNAL3 = 207,    /*!< Internal function signal 3 */
    FUNC_INTERNAL4 = 208,    /*!< Internal function signal 4 */
    FUNC_INTERNAL5 = 209,    /*!< Internal function signal 5 */
    FUNC_INTERNAL6 = 210,    /*!< Internal function signal 6 */
    FUNC_INTERNAL7 = 211,    /*!< Internal function signal 7 */
    FUNC_INTERNAL8 = 212,    /*!< Internal function signal 8 */
    FUNC_INTERNAL9 = 213,    /*!< Internal function signal 9 */
    FUNC_INTERNAL10 = 214,    /*!< Internal function signal 10 */
    FUNC_INTERNAL11 = 215,    /*!< Internal function signal 11 */
    FUNC_INTERNAL12 = 216,    /*!< Internal function signal 12 */
    FUNC_INTERNAL13 = 217,    /*!< Internal function signal 13 */
    FUNC_INTERNAL14 = 218,    /*!< Internal function signal 14 */
    FUNC_INTERNAL15 = 219,    /*!< Internal function signal 15 */
    FUNC_INTERNAL16 = 220,    /*!< Internal function signal 16 */
    FUNC_INTERNAL17 = 221,    /*!< Internal function signal 17 */
    FUNC_CONSTANT = 222,    /*!< Constant function */
    FUNC_INTERNAL18 = 223,    /*!< Internal function signal 18 */
    FUNC_DEBUG0 = 224,    /*!< Debug function 0 */
    FUNC_DEBUG1 = 225,    /*!< Debug function 1 */
    FUNC_DEBUG2 = 226,    /*!< Debug function 2 */
    FUNC_DEBUG3 = 227,    /*!< Debug function 3 */
    FUNC_DEBUG4 = 228,    /*!< Debug function 4 */
    FUNC_DEBUG5 = 229,    /*!< Debug function 5 */
    FUNC_DEBUG6 = 230,    /*!< Debug function 6 */
    FUNC_DEBUG7 = 231,    /*!< Debug function 7 */
    FUNC_DEBUG8 = 232,    /*!< Debug function 8 */
    FUNC_DEBUG9 = 233,    /*!< Debug function 9 */
    FUNC_DEBUG10 = 234,    /*!< Debug function 10 */
    FUNC_DEBUG11 = 235,    /*!< Debug function 11 */
    FUNC_DEBUG12 = 236,    /*!< Debug function 12 */
    FUNC_DEBUG13 = 237,    /*!< Debug function 13 */
    FUNC_DEBUG14 = 238,    /*!< Debug function 14 */
    FUNC_DEBUG15 = 239,    /*!< Debug function 15 */
    FUNC_DEBUG16 = 240,    /*!< Debug function 16 */
    FUNC_DEBUG17 = 241,    /*!< Debug function 17 */
    FUNC_DEBUG18 = 242,    /*!< Debug function 18 */
    FUNC_DEBUG19 = 243,    /*!< Debug function 19 */
    FUNC_DEBUG20 = 244,    /*!< Debug function 20 */
    FUNC_DEBUG21 = 245,    /*!< Debug function 21 */
    FUNC_DEBUG22 = 246,    /*!< Debug function 22 */
    FUNC_DEBUG23 = 247,    /*!< Debug function 23 */
    FUNC_DEBUG24 = 248,    /*!< Debug function 24 */
    FUNC_DEBUG25 = 249,    /*!< Debug function 25 */
    FUNC_DEBUG26 = 250,    /*!< Debug function 26 */
    FUNC_DEBUG27 = 251,    /*!< Debug function 27 */
    FUNC_DEBUG28 = 252,    /*!< Debug function 28 */
    FUNC_DEBUG29 = 253,    /*!< Debug function 29 */
    FUNC_DEBUG30 = 254,    /*!< Debug function 30 */
    FUNC_DEBUG31 = 255,    /*!< Debug function 31 */
    FUNC_MAX = 256,    /*!< Function numbers */
} fpioa_function_t;
/* Pad number settings */
#define FPIOA_NUM_IO    (48)
typedef struct _fpioa_io_config {
    uint32 ch_sel: 8;
    /*!< Channel select from 256 input. */
    uint32 ds: 4;
    /*!< Driving selector. */
    uint32 oe_en: 1;
    /*!< Static output enable, will AND with OE_INV. */
    uint32 oe_inv: 1;
    /*!< Invert output enable. */
    uint32 do_sel: 1;
    /*!< Data output select: 0 for DO, 1 for OE. */
    uint32 do_inv: 1;
    /*!< Invert the result of data output select (DO_SEL). */
    uint32 pu: 1;
    /*!< Pull up enable. 0 for nothing, 1 for pull up. */
    uint32 pd: 1;
    /*!< Pull down enable. 0 for nothing, 1 for pull down. */
    uint32 resv0: 1;
    /*!< Reserved bits. */
    uint32 sl: 1;
    /*!< Slew rate control enable. */
    uint32 ie_en: 1;
    /*!< Static input enable, will AND with IE_INV. */
    uint32 ie_inv: 1;
    /*!< Invert input enable. */
    uint32 di_inv: 1;
    /*!< Invert Data input. */
    uint32 st: 1;
    /*!< Schmitt trigger. */
    uint32 resv1: 7;
    /*!< Reserved bits. */
    uint32 pad_di: 1;
    /*!< Read current IO's data input. */
} __attribute__((packed, aligned(4))) fpioa_io_config_t;
typedef struct _fpioa_tie {
    uint32 en[FUNC_MAX / 32];
    /*!< FPIOA GPIO multiplexer tie enable array */
    uint32 val[FUNC_MAX / 32];
    /*!< FPIOA GPIO multiplexer tie value array */
} __attribute__((packed, aligned(4))) fpioa_tie_t;
typedef struct _fpioa {
    fpioa_io_config_t io[FPIOA_NUM_IO];
    /*!< FPIOA GPIO multiplexer io array */
    fpioa_tie_t tie;
    /*!< FPIOA GPIO multiplexer tie */
} __attribute__((packed, aligned(4))) fpioa_t;
#define FPIOA_BASE_ADDR     (0x502B0000U)
volatile fpioa_t *const fpioa = (volatile fpioa_t *) FPIOA_BASE_ADDR;

int fpioa_get_io_by_function(fpioa_function_t function) {
    int index = 0;
    for (index = 0; index < FPIOA_NUM_IO; index++) {
        if (fpioa->io[index].ch_sel == function)
            return index;
    }

    return -1;
}

typedef enum _fpioa_pull {
    FPIOA_PULL_NONE,      /*!< No Pull */
    FPIOA_PULL_DOWN,      /*!< Pull Down */
    FPIOA_PULL_UP,        /*!< Pull Up */
    FPIOA_PULL_MAX        /*!< Count of pull settings */
} fpioa_pull_t;

int fpioa_set_io_pull(int number, fpioa_pull_t pull) {
    /* Check parameters */
    if (number < 0 || number >= FPIOA_NUM_IO || pull >= FPIOA_PULL_MAX)
        return -1;

    /* Atomic read register */
    fpioa_io_config_t cfg = fpioa->io[number];

    switch (pull) {
        case FPIOA_PULL_NONE:
            cfg.pu = 0;
            cfg.pd = 0;
            break;
        case FPIOA_PULL_DOWN:
            cfg.pu = 0;
            cfg.pd = 1;
            break;
        case FPIOA_PULL_UP:
            cfg.pu = 1;
            cfg.pd = 0;
            break;
        default:
            break;
    }
    /* Atomic write register */
    fpioa->io[number] = cfg;
    return 0;
}

void gpiohs_set_drive_mode(uint8 pin, gpio_drive_mode_t mode) {
    // configASSERT(pin < GPIOHS_MAX_PINNO);
    int io_number = fpioa_get_io_by_function(FUNC_GPIOHS0 + pin);
    // configASSERT(io_number >= 0);

    fpioa_pull_t pull = FPIOA_PULL_NONE;
    uint32 dir = 0;

    switch (mode) {
        case GPIO_DM_INPUT:
            pull = FPIOA_PULL_NONE;
            dir = 0;
            break;
        case GPIO_DM_INPUT_PULL_DOWN:
            pull = FPIOA_PULL_DOWN;
            dir = 0;
            break;
        case GPIO_DM_INPUT_PULL_UP:
            pull = FPIOA_PULL_UP;
            dir = 0;
            break;
        case GPIO_DM_OUTPUT:
            pull = FPIOA_PULL_DOWN;
            dir = 1;
            break;
        default:
            // configASSERT(!"GPIO drive mode is not supported.")
            break;
    }

    fpioa_set_io_pull(io_number, pull);
    volatile uint32 *reg = dir ? gpiohs->output_en.u32 : gpiohs->input_en.u32;
    volatile uint32 *reg_d = !dir ? gpiohs->output_en.u32 : gpiohs->input_en.u32;
    set_gpio_bit(reg_d, pin, 0);
    set_gpio_bit(reg, pin, 1);
}

static void sd_lowlevel_init(uint8 spi_index) {
    gpiohs_set_drive_mode(7, GPIO_DM_OUTPUT);
    // spi_set_clk_rate(SPI_DEVICE_0, 200000);     /*set clk rate*/
}

typedef enum _spi_device_num {
    SPI_DEVICE_0,
    SPI_DEVICE_1,
    SPI_DEVICE_2,
    SPI_DEVICE_3,
    SPI_DEVICE_MAX,
} spi_device_num_t;
typedef enum _spi_work_mode {
    SPI_WORK_MODE_0,
    SPI_WORK_MODE_1,
    SPI_WORK_MODE_2,
    SPI_WORK_MODE_3,
} spi_work_mode_t;
typedef enum _spi_frame_format {
    SPI_FF_STANDARD,
    SPI_FF_DUAL,
    SPI_FF_QUAD,
    SPI_FF_OCTAL
} spi_frame_format_t;
typedef struct _spi {
    /* SPI Control Register 0                                    (0x00)*/
    volatile uint32 ctrlr0;
    /* SPI Control Register 1                                    (0x04)*/
    volatile uint32 ctrlr1;
    /* SPI Enable Register                                       (0x08)*/
    volatile uint32 ssienr;
    /* SPI Microwire Control Register                            (0x0c)*/
    volatile uint32 mwcr;
    /* SPI Slave Enable Register                                 (0x10)*/
    volatile uint32 ser;
    /* SPI Baud Rate Select                                      (0x14)*/
    volatile uint32 baudr;
    /* SPI Transmit FIFO Threshold Level                         (0x18)*/
    volatile uint32 txftlr;
    /* SPI Receive FIFO Threshold Level                          (0x1c)*/
    volatile uint32 rxftlr;
    /* SPI Transmit FIFO Level Register                          (0x20)*/
    volatile uint32 txflr;
    /* SPI Receive FIFO Level Register                           (0x24)*/
    volatile uint32 rxflr;
    /* SPI Status Register                                       (0x28)*/
    volatile uint32 sr;
    /* SPI Interrupt Mask Register                               (0x2c)*/
    volatile uint32 imr;
    /* SPI Interrupt Status Register                             (0x30)*/
    volatile uint32 isr;
    /* SPI Raw Interrupt Status Register                         (0x34)*/
    volatile uint32 risr;
    /* SPI Transmit FIFO Overflow Interrupt Clear Register       (0x38)*/
    volatile uint32 txoicr;
    /* SPI Receive FIFO Overflow Interrupt Clear Register        (0x3c)*/
    volatile uint32 rxoicr;
    /* SPI Receive FIFO Underflow Interrupt Clear Register       (0x40)*/
    volatile uint32 rxuicr;
    /* SPI Multi-Master Interrupt Clear Register                 (0x44)*/
    volatile uint32 msticr;
    /* SPI Interrupt Clear Register                              (0x48)*/
    volatile uint32 icr;
    /* SPI DMA Control Register                                  (0x4c)*/
    volatile uint32 dmacr;
    /* SPI DMA Transmit Data Level                               (0x50)*/
    volatile uint32 dmatdlr;
    /* SPI DMA Receive Data Level                                (0x54)*/
    volatile uint32 dmardlr;
    /* SPI Identification Register                               (0x58)*/
    volatile uint32 idr;
    /* SPI DWC_ssi component version                             (0x5c)*/
    volatile uint32 ssic_version_id;
    /* SPI Data Register 0-36                                    (0x60 -- 0xec)*/
    volatile uint32 dr[36];
    /* SPI RX Sample Delay Register                              (0xf0)*/
    volatile uint32 rx_sample_delay;
    /* SPI SPI Control Register                                  (0xf4)*/
    volatile uint32 spi_ctrlr0;
    /* reserved                                                  (0xf8)*/
    volatile uint32 resv;
    /* SPI XIP Mode bits                                         (0xfc)*/
    volatile uint32 xip_mode_bits;
    /* SPI XIP INCR transfer opcode                              (0x100)*/
    volatile uint32 xip_incr_inst;
    /* SPI XIP WRAP transfer opcode                              (0x104)*/
    volatile uint32 xip_wrap_inst;
    /* SPI XIP Control Register                                  (0x108)*/
    volatile uint32 xip_ctrl;
    /* SPI XIP Slave Enable Register                             (0x10c)*/
    volatile uint32 xip_ser;
    /* SPI XIP Receive FIFO Overflow Interrupt Clear Register    (0x110)*/
    volatile uint32 xrxoicr;
    /* SPI XIP time out register for continuous transfers        (0x114)*/
    volatile uint32 xip_cnt_time_out;
    volatile uint32 endian;
} __attribute__((packed, aligned(4))) spi_t;
#define SPI0_BASE_ADDR      (0x52000000U)
#define SPI1_BASE_ADDR      (0x53000000U)
#define SPI_SLAVE_BASE_ADDR (0x50240000U)
#define SPI3_BASE_ADDR      (0x54000000U)
volatile spi_t *const spi[4] =
        {
                (volatile spi_t *) SPI0_BASE_ADDR,
                (volatile spi_t *) SPI1_BASE_ADDR,
                (volatile spi_t *) SPI_SLAVE_BASE_ADDR,
                (volatile spi_t *) SPI3_BASE_ADDR};

void spi_init(spi_device_num_t spi_num, spi_work_mode_t work_mode, spi_frame_format_t frame_format,
              uint64 data_bit_length, uint32 endian) {
    // configASSERT(data_bit_length >= 4 && data_bit_length <= 32);
    // configASSERT(spi_num < SPI_DEVICE_MAX && spi_num != 2);
    // spi_clk_init(spi_num);

    // uint8 dfs_offset, frf_offset, work_mode_offset;
    uint8 dfs_offset = 0;
    uint8 frf_offset = 0;
    uint8 work_mode_offset = 0;
    switch (spi_num) {
        case 0:
        case 1:
            dfs_offset = 16;
            frf_offset = 21;
            work_mode_offset = 6;
            break;
        case 2:
            // configASSERT(!"Spi Bus 2 Not Support!");
            break;
        case 3:
        default:
            dfs_offset = 0;
            frf_offset = 22;
            work_mode_offset = 8;
            break;
    }

    switch (frame_format) {
        case SPI_FF_DUAL:
            // configASSERT(data_bit_length % 2 == 0);
            break;
        case SPI_FF_QUAD:
            // configASSERT(data_bit_length % 4 == 0);
            break;
        case SPI_FF_OCTAL:
            // configASSERT(data_bit_length % 8 == 0);
            break;
        default:
            break;
    }
    volatile spi_t *spi_adapter = spi[spi_num];
    if (spi_adapter->baudr == 0)
        spi_adapter->baudr = 0x14;
    spi_adapter->imr = 0x00;
    spi_adapter->dmacr = 0x00;
    spi_adapter->dmatdlr = 0x10;
    spi_adapter->dmardlr = 0x00;
    spi_adapter->ser = 0x00;
    spi_adapter->ssienr = 0x00;
    spi_adapter->ctrlr0 =
            (work_mode << work_mode_offset) | (frame_format << frf_offset) | ((data_bit_length - 1) << dfs_offset);
    spi_adapter->spi_ctrlr0 = 0;
    spi_adapter->endian = endian;
}

typedef enum _spi_chip_select {
    SPI_CHIP_SELECT_0,
    SPI_CHIP_SELECT_1,
    SPI_CHIP_SELECT_2,
    SPI_CHIP_SELECT_3,
    SPI_CHIP_SELECT_MAX,
} spi_chip_select_t;

static void spi_set_tmod(uint8 spi_num, uint32 tmod) {
    // configASSERT(spi_num < SPI_DEVICE_MAX);
    volatile spi_t *spi_handle = spi[spi_num];
    uint8 tmod_offset = 0;
    switch (spi_num) {
        case 0:
        case 1:
        case 2:
            tmod_offset = 8;
            break;
        case 3:
        default:
            tmod_offset = 10;
            break;
    }
    set_bit(&spi_handle->ctrlr0, 3 << tmod_offset, tmod << tmod_offset);
}

typedef enum _spi_transfer_mode {
    SPI_TMOD_TRANS_RECV,
    SPI_TMOD_TRANS,
    SPI_TMOD_RECV,
    SPI_TMOD_EEROM
} spi_transfer_mode_t;
typedef enum _spi_transfer_width {
    SPI_TRANS_CHAR = 0x1,
    SPI_TRANS_SHORT = 0x2,
    SPI_TRANS_INT = 0x4,
} spi_transfer_width_t;

static spi_transfer_width_t spi_get_frame_size(uint64 data_bit_length) {
    if (data_bit_length < 8)
        return SPI_TRANS_CHAR;
    else if (data_bit_length < 16)
        return SPI_TRANS_SHORT;
    return SPI_TRANS_INT;
}

void
spi_send_data_normal(spi_device_num_t spi_num, spi_chip_select_t chip_select, const uint8 *tx_buff, uint64 tx_len) {
    // configASSERT(spi_num < SPI_DEVICE_MAX && spi_num != 2);

    uint64 index, fifo_len;
    spi_set_tmod(spi_num, SPI_TMOD_TRANS);

    volatile spi_t *spi_handle = spi[spi_num];

    // uint8 dfs_offset;
    uint8 dfs_offset = 0;
    switch (spi_num) {
        case 0:
        case 1:
            dfs_offset = 16;
            break;
        case 2:
            // configASSERT(!"Spi Bus 2 Not Support!");
            break;
        case 3:
        default:
            dfs_offset = 0;
            break;
    }
    uint32 data_bit_length = (spi_handle->ctrlr0 >> dfs_offset) & 0x1F;
    spi_transfer_width_t frame_width = spi_get_frame_size(data_bit_length);

    uint8 v_misalign_flag = 0;
    uint32 v_send_data;
    if ((uintptr_t) tx_buff % frame_width)
        v_misalign_flag = 1;

    spi_handle->ssienr = 0x01;
    spi_handle->ser = 1U << chip_select;
    uint32 i = 0;
    while (tx_len) {
        fifo_len = 32 - spi_handle->txflr;
        fifo_len = fifo_len < tx_len ? fifo_len : tx_len;
        switch (frame_width) {
            case SPI_TRANS_INT:
                fifo_len = fifo_len / 4 * 4;
                if (v_misalign_flag) {
                    for (index = 0; index < fifo_len; index += 4) {
                        memcpy(&v_send_data, tx_buff + i, 4);
                        spi_handle->dr[0] = v_send_data;
                        i += 4;
                    }
                } else {
                    for (index = 0; index < fifo_len / 4; index++)
                        spi_handle->dr[0] = ((uint32 *) tx_buff)[i++];
                }
                break;
            case SPI_TRANS_SHORT:
                fifo_len = fifo_len / 2 * 2;
                if (v_misalign_flag) {
                    for (index = 0; index < fifo_len; index += 2) {
                        memcpy(&v_send_data, tx_buff + i, 2);
                        spi_handle->dr[0] = v_send_data;
                        i += 2;
                    }
                } else {
                    for (index = 0; index < fifo_len / 2; index++)
                        spi_handle->dr[0] = ((uint16 *) tx_buff)[i++];
                }
                break;
            default:
                for (index = 0; index < fifo_len; index++)
                    spi_handle->dr[0] = tx_buff[i++];
                break;
        }
        tx_len -= fifo_len;
    }
    while ((spi_handle->sr & 0x05) != 0x04);
    spi_handle->ser = 0x00;
    spi_handle->ssienr = 0x00;
}

void spi_send_data_standard(spi_device_num_t spi_num, spi_chip_select_t chip_select, const uint8 *cmd_buff,
                            uint64 cmd_len, const uint8 *tx_buff, uint64 tx_len) {
    // configASSERT(spi_num < SPI_DEVICE_MAX && spi_num != 2);
    uint8 *v_buf = k_malloc(cmd_len + tx_len);
    uint64 i;
    for (i = 0; i < cmd_len; i++)
        v_buf[i] = cmd_buff[i];
    for (i = 0; i < tx_len; i++)
        v_buf[cmd_len + i] = tx_buff[i];

    spi_send_data_normal(spi_num, chip_select, v_buf, cmd_len + tx_len);
    k_free((void *) v_buf);
}

void spi_receive_data_standard(spi_device_num_t spi_num, spi_chip_select_t chip_select, const uint8 *cmd_buff,
                               uint64 cmd_len, uint8 *rx_buff, uint64 rx_len) {
    // configASSERT(spi_num < SPI_DEVICE_MAX && spi_num != 2);
    uint64 index, fifo_len;
    if (cmd_len == 0)
        spi_set_tmod(spi_num, SPI_TMOD_RECV);
    else
        spi_set_tmod(spi_num, SPI_TMOD_EEROM);
    volatile spi_t *spi_handle = spi[spi_num];

    // uint8 dfs_offset;
    uint8 dfs_offset = 0;
    switch (spi_num) {
        case 0:
        case 1:
            dfs_offset = 16;
            break;
        case 2:
            // configASSERT(!"Spi Bus 2 Not Support!");
            break;
        case 3:
        default:
            dfs_offset = 0;
            break;
    }
    uint32 data_bit_length = (spi_handle->ctrlr0 >> dfs_offset) & 0x1F;
    spi_transfer_width_t frame_width = spi_get_frame_size(data_bit_length);

    uint32 i = 0;
    uint64 v_cmd_len = cmd_len / frame_width;
    uint32 v_rx_len = rx_len / frame_width;

    spi_handle->ctrlr1 = (uint32) (v_rx_len - 1);
    spi_handle->ssienr = 0x01;

    while (v_cmd_len) {
        fifo_len = 32 - spi_handle->txflr;
        fifo_len = fifo_len < v_cmd_len ? fifo_len : v_cmd_len;
        switch (frame_width) {
            case SPI_TRANS_INT:
                for (index = 0; index < fifo_len; index++)
                    spi_handle->dr[0] = ((uint32 *) cmd_buff)[i++];
                break;
            case SPI_TRANS_SHORT:
                for (index = 0; index < fifo_len; index++)
                    spi_handle->dr[0] = ((uint16 *) cmd_buff)[i++];
                break;
            default:
                for (index = 0; index < fifo_len; index++)
                    spi_handle->dr[0] = cmd_buff[i++];
                break;
        }
        spi_handle->ser = 1U << chip_select;
        v_cmd_len -= fifo_len;
    }

    if (cmd_len == 0) {
        spi_handle->dr[0] = 0xffffffff;
        spi_handle->ser = 1U << chip_select;
    }

    i = 0;
    while (v_rx_len) {
        fifo_len = spi_handle->rxflr;
        fifo_len = fifo_len < v_rx_len ? fifo_len : v_rx_len;
        switch (frame_width) {
            case SPI_TRANS_INT:
                for (index = 0; index < fifo_len; index++)
                    ((uint32 *) rx_buff)[i++] = spi_handle->dr[0];
                break;
            case SPI_TRANS_SHORT:
                for (index = 0; index < fifo_len; index++)
                    ((uint16 *) rx_buff)[i++] = (uint16) spi_handle->dr[0];
                break;
            default:
                for (index = 0; index < fifo_len; index++)
                    rx_buff[i++] = (uint8) spi_handle->dr[0];
                break;
        }

        v_rx_len -= fifo_len;
    }

    spi_handle->ser = 0x00;
    spi_handle->ssienr = 0x00;
}

static void sd_write_data(uint8 const *data_buff, uint32 length) {
    spi_init(SPI_DEVICE_0, SPI_WORK_MODE_0, SPI_FF_STANDARD, 8, 0);
    spi_send_data_standard(SPI_DEVICE_0, SPI_CHIP_SELECT_3, NULL, 0, data_buff, length);
}

static void sd_read_data(uint8 *data_buff, uint32 length) {
    spi_init(SPI_DEVICE_0, SPI_WORK_MODE_0, SPI_FF_STANDARD, 8, 0);
    spi_receive_data_standard(SPI_DEVICE_0, SPI_CHIP_SELECT_3, NULL, 0, data_buff, length);
}

/*
 * @brief  Send 5 bytes command to the SD card.
 * @param  Cmd: The user expected command to send to SD card.
 * @param  Arg: The command argument.
 * @param  Crc: The CRC.
 * @retval None
 */
static void sd_send_cmd(uint8 cmd, uint32 arg, uint8 crc) {
    uint8 frame[6];
    frame[0] = (cmd | 0x40);
    frame[1] = (uint8) (arg >> 24);
    frame[2] = (uint8) (arg >> 16);
    frame[3] = (uint8) (arg >> 8);
    frame[4] = (uint8) (arg);
    frame[5] = (crc);


    SD_CS_LOW();



    sd_write_data(frame, 6);
}

static void sd_end_cmd(void) {
    uint8 frame[1] = {0xFF};
    /*!< SD chip select high */
    SD_CS_HIGH();
    /*!< Send the Cmd bytes */
    sd_write_data(frame, 1);
}

/*
 * Be noticed: all commands & responses below
 * 		are in SPI mode format. May differ from
 * 		what they are in SD mode.
 */

#define SD_CMD0    0
#define SD_CMD8    8
#define SD_CMD58    58        // READ_OCR
#define SD_CMD55    55        // APP_CMD
#define SD_ACMD41    41        // SD_SEND_OP_COND
#define SD_CMD16    16        // SET_BLOCK_SIZE
#define SD_CMD17    17        // READ_SINGLE_BLOCK
#define SD_CMD24    24        // WRITE_SINGLE_BLOCK
#define SD_CMD13    13        // SEND_STATUS

/*
 * Read sdcard response in R1 type.
 */
static uint8 sd_get_response_R1(void) {
    uint8 result;
    uint16 timeout = 0xff;

    while (timeout--) {
        sd_read_data(&result, 1);
        if (result != 0xff)
            return result;
    }

    // timeout!
    return 0xff;
}

/*
 * Read the rest of R3 response
 * Be noticed: frame should be at least 4-byte long
 */
static void sd_get_response_R3_rest(uint8 *frame) {
    sd_read_data(frame, 4);
}

/*
 * Read the rest of R7 response
 * Be noticed: frame should be at least 4-byte long
 */
static void sd_get_response_R7_rest(uint8 *frame) {
    sd_read_data(frame, 4);
}

static int switch_to_SPI_mode(void) {
    int timeout = 0xff;

    while (--timeout) {
        sd_send_cmd(SD_CMD0, 0, 0x95);
        uint64 result = sd_get_response_R1();
        sd_end_cmd();

        if (0x01 == result) break;
    }
    if (0 == timeout) {
        printf("SD_CMD0 failed\n");
        return 0xff;
    }

    return 0;
}

// verify supply voltage range
static int verify_operation_condition(void) {
    uint64 result;

    // Stores the response reversely.
    // That means
    // frame[2] - VCA
    // frame[3] - Check Pattern
    uint8 frame[4];

    sd_send_cmd(SD_CMD8, 0x01aa, 0x87);
    result = sd_get_response_R1();
    sd_get_response_R7_rest(frame);
    sd_end_cmd();

    if (0x09 == result) {
        printf("invalid CRC for CMD8\n");
        return 0xff;
    } else if (0x01 == result && 0x01 == (frame[2] & 0x0f) && 0xaa == frame[3]) {
        return 0x00;
    }

    printf("verify_operation_condition() fail!\n");
    return 0xff;
}

// read OCR register to check if the voltage range is valid
// this step is not mandotary, but I advise to use it
static int read_OCR(void) {
    uint64 result;
    uint8 ocr[4];

    int timeout;

    timeout = 0xff;
    while (--timeout) {
        sd_send_cmd(SD_CMD58, 0, 0);
        result = sd_get_response_R1();
        sd_get_response_R3_rest(ocr);
        sd_end_cmd();

        if (
                0x01 == result && // R1 response in idle status
                (ocr[1] & 0x1f) && (ocr[2] & 0x80)    // voltage range valid
                ) {
            return 0;
        }
    }

    // timeout!
    printf("read_OCR() timeout!\n");
    printf("result = %d\n", result);
    return 0xff;
}

// send ACMD41 to tell sdcard to finish initializing
static int set_SDXC_capacity(void) {
    uint8 result = 0xff;

    int timeout = 0xfff;
    while (--timeout) {
        sd_send_cmd(SD_CMD55, 0, 0);
        result = sd_get_response_R1();
        sd_end_cmd();
        if (0x01 != result) {
            printf("SD_CMD55 fail! result = %d\n", result);
            return 0xff;
        }

        sd_send_cmd(SD_ACMD41, 0x40000000, 0);
        result = sd_get_response_R1();
        sd_end_cmd();
        if (0 == result) {
            return 0;
        }
    }

    // timeout!
    printf("set_SDXC_capacity() timeout!\n");
    printf("result = %d\n", result);
    return 0xff;
}

// Used to differ whether sdcard is SDSC type.
static int is_standard_sd = 0;
#define BSIZE 512

// check OCR register to see the type of sdcard,
// thus determine whether block size is suitable to buffer size
static int check_block_size(void) {
    uint8 result = 0xff;
    uint8 ocr[4];

    int timeout = 0xff;
    while (timeout--) {
        sd_send_cmd(SD_CMD58, 0, 0);
        result = sd_get_response_R1();
        sd_get_response_R3_rest(ocr);
        sd_end_cmd();

        if (0 == result) {
            if (ocr[0] & 0x40) {
                printf("SDHC/SDXC detected\n");
                if (512 != BSIZE) {
                    printf("BSIZE != 512\n");
                    return 0xff;
                }

                is_standard_sd = 0;
            } else {
                printf("SDSC detected, setting block size\n");

                // setting SD card block size to BSIZE
                int timeout = 0xff;
                int result = 0xff;
                while (--timeout) {
                    sd_send_cmd(SD_CMD16, BSIZE, 0);
                    result = sd_get_response_R1();
                    sd_end_cmd();

                    if (0 == result) break;
                }
                if (0 == timeout) {
                    printf("check_OCR(): fail to set block size");
                    return 0xff;
                }

                is_standard_sd = 1;
            }

            return 0;
        }
    }

    // timeout!
    printf("check_OCR() timeout!\n");
    printf("result = %d\n", result);
    return 0xff;
}

/*
 * @brief  Initializes the SD/SD communication.
 * @param  None
 * @retval The SD Response:
 *         - 0xFF: Sequence failed
 *         - 0: Sequence succeed
 */
static int sd_init(void) {
    uint8 frame[10];

    sd_lowlevel_init(0);
    //SD_CS_HIGH();
    SD_CS_LOW();

    // send dummy bytes for 80 clock cycles
    for (int i = 0; i < 10; i++)
        frame[i] = 0xff;
    sd_write_data(frame, 10);

    if (0 != switch_to_SPI_mode())
        return 0xff;
    if (0 != verify_operation_condition())
        return 0xff;
    if (0 != read_OCR())
        return 0xff;
    if (0 != set_SDXC_capacity())
        return 0xff;
    if (0 != check_block_size())
        return 0xff;

    return 0;
}

struct spinlock;

struct spinlock {
    uint locked;       // Is the lock held?

    // For debugging:
    char *name;        // Name of lock.
    struct cpu *cpu;   // The cpu holding the lock.
};

// Long-term locks for processes
struct sleeplock {
    uint locked;       // Is the lock held?
    struct spinlock lk; // spinlock protecting this sleep lock

    // For debugging:
    char *name;        // Name of lock.
    int pid;           // Process holding lock
};

static struct sleeplock sdcard_lock;


void
initlock(struct spinlock *lk, char *name) {
    lk->name = name;
    lk->locked = 0;
    lk->cpu = 0;
}

void
initsleeplock(struct sleeplock *lk, char *name) {
    initlock(&lk->lk, "sleep lock");
    lk->name = name;
    lk->locked = 0;
    lk->pid = 0;
}

void sdcard_init(void) {
    int result = sd_init();
    initsleeplock(&sdcard_lock, "sdcard");

    if (0 != result) {
        printf("sdcard_init failed");
    }
#ifdef DEBUG
    printf("sdcard_init\n");
#endif
}


void sdcard_read_sector(uint8 *buf, int sectorno) {
    uint8 result;
    uint32 address;
    uint8 dummy_crc[2];

#ifdef DEBUG
    printf("sdcard_read_sector()\n");
#endif

    if (is_standard_sd) {
        address = sectorno << 9;
    } else {
        address = sectorno;
    }

    // enter critical section!
//    acquiresleep(&sdcard_lock);

    sd_send_cmd(SD_CMD17, address, 0);
    result = sd_get_response_R1();

    if (0 != result) {
//        releasesleep(&sdcard_lock);
        printf("sdcard: fail to read");
    }

    int timeout = 0xffffff;
    while (--timeout) {
        sd_read_data(&result, 1);
        if (0xfe == result) break;
    }
    if (0 == timeout) {
        printf("sdcard: timeout waiting for reading");
    }
    sd_read_data(buf, BSIZE);
    sd_read_data(dummy_crc, 2);

    sd_end_cmd();

//    releasesleep(&sdcard_lock);
    // leave critical section!
}

void sdcard_write_sector(uint8 *buf, int sectorno) {
    uint32 address;
    static uint8 const START_BLOCK_TOKEN = 0xfe;
    uint8 dummy_crc[2] = {0xff, 0xff};

#ifdef DEBUG
    printf("sdcard_write_sector()\n");
#endif

    if (is_standard_sd) {
        address = sectorno << 9;
    } else {
        address = sectorno;
    }

    // enter critical section!
//    acquiresleep(&sdcard_lock);



    sd_send_cmd(SD_CMD24, address, 0);



    if (0 != sd_get_response_R1()) {
//        releasesleep(&sdcard_lock);
        printf("sdcard: fail to write");
    }

    // sending data to be written
    sd_write_data(&START_BLOCK_TOKEN, 1);
    sd_write_data(buf, BSIZE);
    sd_write_data(dummy_crc, 2);

    // waiting for sdcard to finish programming
    uint8 result;
    int timeout = 0xfff;
    while (--timeout) {
        sd_read_data(&result, 1);
        if (0x05 == (result & 0x1f)) {
            break;
        }
    }
    if (0 == timeout) {
//        releasesleep(&sdcard_lock);
        printf("sdcard: invalid response token");
    }

    timeout = 0xffffff;
    while (--timeout) {
        sd_read_data(&result, 1);
        if (0 != result) break;
    }
    if (0 == timeout) {
//        releasesleep(&sdcard_lock);
        printf("sdcard: timeout waiting for response");
    }
    sd_end_cmd();

    // send SD_CMD13 to check if writing is correctly done
    uint8 error_code = 0xff;
    sd_send_cmd(SD_CMD13, 0, 0);
    result = sd_get_response_R1();
    sd_read_data(&error_code, 1);
    sd_end_cmd();
    if (0 != result || 0 != error_code) {
//        releasesleep(&sdcard_lock);
        printf("result: %x\n", result);
        printf("error_code: %x\n", error_code);
        printf("sdcard: an error occurs when writing");
    }

//    releasesleep(&sdcard_lock);
    // leave critical section!
}


// A simple test for sdcard read/write test
void test_sdcard(void) {
    uint8 buf[BSIZE];

    for (int sec = 0; sec < 5; sec++) {
        for (int i = 0; i < BSIZE; i++) {
            buf[i] = 0xaa;        // data to be written
        }

        sdcard_write_sector(buf, sec);

        for (int i = 0; i < BSIZE; i++) {
            buf[i] = 0xff;        // fill in junk
        }

        sdcard_read_sector(buf, sec);
        for (int i = 0; i < BSIZE; i++) {
            if (0 == i % 16) {
                printf("\n");
            }

            printf("%x ", buf[i]);
        }
        printf("\n");
    }

    while (1);
}
