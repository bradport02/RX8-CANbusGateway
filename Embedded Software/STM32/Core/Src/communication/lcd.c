/*
 * lcd.c
 * Created on: 23 Mar 2026
 * Author: Bradley Port
 * Royal Holloway University of London
 * EE3000
 *
 * STM32 HAL port of OEM RX-8 LCD driver
 */
#include "lcd.h"
#include "vehicle/time.h"

static SPI_HandleTypeDef *_hspi;
static TIM_HandleTypeDef *_htim;

// Shadow arrays — match Arduino iconArray[0] and sevenSegmentArray[0]
static uint8_t iconData[LCD_ICON_COUNT];
static uint8_t iconDataFlushed[LCD_ICON_COUNT];
static uint8_t sevenSegData[7];
extern uint8_t lastMinute; //defined in main

//RTC Data
extern RTC_HandleTypeDef hrtc;
extern RTC_TimeTypeDef sTime;
extern RTC_DateTypeDef sDate;
extern uint8_t lastMinute;

extern uint8_t climateReceived;

uint8_t saved[4] = {0};

// Icon register addresses — matches Arduino iconArray[1]
static const uint8_t iconAddr[LCD_ICON_COUNT] = {
    0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96,
    0x97, 0x98, 0x99, 0x9A, 0x9B, 0x9C, 0x9D
};

// -------------------------------------------------------
// Pin helpers — replace Arduino digitalWrite
// -------------------------------------------------------
static inline void AC_Low(void)  { HAL_GPIO_WritePin(LCD_AC_PORT, LCD_AC_PIN, GPIO_PIN_RESET); }
static inline void AC_High(void) { HAL_GPIO_WritePin(LCD_AC_PORT, LCD_AC_PIN, GPIO_PIN_SET); }
static inline void RS_Low(void)  { HAL_GPIO_WritePin(LCD_RS_PORT, LCD_RS_PIN, GPIO_PIN_RESET); }
static inline void RS_High(void) { HAL_GPIO_WritePin(LCD_RS_PORT, LCD_RS_PIN, GPIO_PIN_SET); }
static inline void CS_Low(void)  { HAL_GPIO_WritePin(LCD_CS_PORT, LCD_CS_PIN, GPIO_PIN_RESET); }
static inline void CS_High(void) { HAL_GPIO_WritePin(LCD_CS_PORT, LCD_CS_PIN, GPIO_PIN_SET); }

// -------------------------------------------------------
// SPI byte transfer — replaces SPI.transfer()
// -------------------------------------------------------
static void SPI_Transfer(uint8_t byte)
{
    HAL_SPI_Transmit(_hspi, &byte, 1, HAL_MAX_DELAY);
}

// -------------------------------------------------------
// Init — store handles, start PWM, run startup sequence
// -------------------------------------------------------
void LCD_Init(SPI_HandleTypeDef *hspi, TIM_HandleTypeDef *htim)
{
    _hspi = hspi;
    _htim = htim;

    memset(iconData, 0, sizeof(iconData));
    memset(sevenSegData, 0, sizeof(sevenSegData));

    // Enable the display — permanently HIGH
    HAL_GPIO_WritePin(GPIOD, LCD_EN_Pin, GPIO_PIN_SET);  // ← add this
    LCD_SetBacklight(0);
    LCD_Startup();
    LCD_SetTime();
    LCD_SetBacklight(100);
}

// -------------------------------------------------------
// Backlight PWM — 0–100%
// TIM2 CH2 on PB3 → MCP1402T → 8V LCD supply
// -------------------------------------------------------
void LCD_SetBacklight(uint8_t percent)
{
    if (percent > 100) percent = 100;
    uint32_t duty = (uint32_t)(percent * 99 / 100); // 75% → compare = 74
    __HAL_TIM_SET_COMPARE(_htim, LCD_PWM_CHANNEL, duty);
    HAL_TIM_PWM_Start(_htim, LCD_PWM_CHANNEL);
}

// -------------------------------------------------------
// Send a command byte (AC=LOW, RS=LOW)
// Matches Arduino: MODE0=LOW, MODE1=LOW
// -------------------------------------------------------
void LCD_SendCommand(uint8_t cmd)
{
    AC_Low();
    RS_Low();
    CS_Low();
    SPI_Transfer(cmd);
    CS_High();
}

// -------------------------------------------------------
// Send a data byte (AC=HIGH, RS=LOW)
// Matches Arduino: MODE0=HIGH, MODE1=LOW in data sections
// -------------------------------------------------------
void LCD_SendData(uint8_t data)
{
    AC_High();
    RS_Low();
    CS_Low();
    SPI_Transfer(data);
    CS_High();
    DWT_Delay_us(42);   // match Arduino delayMicroseconds(42)
    //HAL_Delay(5);
    AC_Low();
}

// -------------------------------------------------------
// Set a single icon register directly
// -------------------------------------------------------
void LCD_SetICON(uint8_t address, uint8_t value)
{
    AC_Low();
    RS_Low();
    CS_Low();
    SPI_Transfer(address);
    CS_High();

    RS_High();
    CS_Low();
    SPI_Transfer(value);
    CS_High();
    Delay_us(42);
    //HAL_Delay(5);
    RS_Low();
}

// -------------------------------------------------------
// Flush all icon shadow registers to display then clear
// -------------------------------------------------------
void LCD_FlushIcons(void)
{
    for (uint8_t i = 0; i < LCD_ICON_COUNT; i++)
    {
        if (iconData[i] == iconDataFlushed[i])
            continue;  // skip — hardware already correct

        AC_Low();
        RS_Low();
        CS_Low();
        SPI_Transfer(iconAddr[i]);
        CS_High();

        RS_High();
        CS_Low();
        SPI_Transfer(iconData[i]);
        CS_High();
        //HAL_Delay(5);
        DWT_Delay_us(42);
        RS_Low();

        iconDataFlushed[i] = iconData[i];  // update the flushed mirror
    }
}

// -------------------------------------------------------
// Flush seven segment CGRAM to display
// -------------------------------------------------------
void LCD_FlushSevenSegment(void)
{
    // Set CGRAM address — AC_High matches Arduino MODE0_HIGH
    RS_Low();
    AC_High();
    CS_Low();
    SPI_Transfer(0x00);
    CS_High();
    AC_Low();

    for (uint8_t i = 0; i < 7; i++)
    {
        RS_High();   // ✅ matches Arduino MODE1_HIGH
        CS_Low();
        SPI_Transfer(sevenSegData[i]);
        CS_High();
        DWT_Delay_us(42);
        //HAL_Delay(4);
        RS_Low();
        //sevenSegData[i] = 0x00;
    }

    // lcdCgramToDdram(0x00, 0x8C) — Arduino: AC_LOW RS_LOW for address, RS_HIGH for data
    AC_Low(); RS_Low();
    CS_Low();
    SPI_Transfer(0x8C);
    CS_High();
    RS_High();
    CS_Low();
    SPI_Transfer(0x00);
    CS_High();
    //HAL_Delay(5);
    DWT_Delay_us(42);
    CS_Low(); RS_Low();

    // lcdCgramToDdram(0x00, 0x8D)
    AC_Low();
    RS_Low();
    CS_Low();
    SPI_Transfer(0x8D); CS_High();
    RS_High();
    CS_Low();
    SPI_Transfer(0x00); CS_High();
    //HAL_Delay(5);
    DWT_Delay_us(42);
    CS_Low(); RS_Low();
}

// -------------------------------------------------------
// Startup sequence — matches Arduino lcdDisplayStartUp()
// -------------------------------------------------------
void LCD_Startup(void)
{
    AC_Low();
    RS_Low();

    //HAL_Delay(50);
    DWT_Delay_us(42);
    LCD_SendCommand(0x08);
    //HAL_Delay(5);
    DWT_Delay_us(42);
    LCD_Clear();
    //HAL_Delay(5);
    DWT_Delay_us(42);
    LCD_Home();
    //HAL_Delay(5);
    DWT_Delay_us(42);
    LCD_SendCommand(0x0C);
    LCD_SendCommand(0x00);
    LCD_SendCommand(0x06);
    LCD_SendCommand(0x46);
    LCD_SendCommand(0x14);
    HAL_Delay(5);

    // Full hardware clear on startup only
    memset(iconData, 0, sizeof(iconData));
    memset(iconDataFlushed, 0xFF, sizeof(iconDataFlushed));
    for (uint8_t i = 0x90; i < 0x9F; i++)
    {
        LCD_SendCommand(i);
        AC_High(); RS_Low();
        CS_Low();
        SPI_Transfer(0x00);
        CS_High();
        //HAL_Delay(5);
        DWT_Delay_us(42);
        AC_Low();
    }

    LCD_Reset();
    //HAL_Delay(5);
    DWT_Delay_us(42);
    LCD_SetDivider();
    LCD_FlushIcons();
}

void LCD_Clear(void)  { LCD_SendCommand(0x01); }
void LCD_Home(void)   { LCD_SendCommand(0x02); }

// -------------------------------------------------------
// Reset icon display — write 0x00 to all icon registers
// -------------------------------------------------------
void LCD_ResetIcons(void)
{
	memset(iconData, 0, sizeof(iconData));
}

// -------------------------------------------------------
// Reset LCD text to spaces
// -------------------------------------------------------
void LCD_Reset(void)
{
    LCD_SendCommand(0x80);
    for (uint8_t i = 0; i < 12; i++)
    {
        AC_High();   // ✅ matches Arduino MODE0_HIGH
        RS_Low();
        CS_Low();
        SPI_Transfer(0x20);
        CS_High();
        DWT_Delay_us(42);
        //HAL_Delay(5);
        AC_Low();
    }
}

// -------------------------------------------------------
// Print string — up to 12 characters, space padded
// -------------------------------------------------------
void LCD_Print(const char *text)
{
    char buf[13] = {0};
    strncpy(buf, text, 12);
    while (strlen(buf) < 12)
        strcat(buf, " ");

    LCD_SendCommand(0x80);

    for (uint8_t i = 0; i < 12; i++)
    {
        RS_High();
        AC_Low();
        CS_Low();
        SPI_Transfer((uint8_t)buf[i]);
        CS_High();
        DWT_Delay_us(42);
        //HAL_Delay(4);
        RS_Low();
    }
}

// -------------------------------------------------------
// Fan speed (0–7) — updates icon shadow array
// -------------------------------------------------------
void LCD_SetFanSpeed(uint8_t speed)
{
    switch (speed)
    {
    	case 0: iconData[9]  += 0x02; break;
        case 1: iconData[9]  += 0x06; break;
        case 2: iconData[9]  += 0x0E; break;
        case 3: iconData[9]  += 0x1E; break;
        case 4: iconData[8]  += 0x01; iconData[9]  += 0x1E; break;
        case 5: iconData[8]  += 0x05; iconData[9]  += 0x1E; break;
        case 6: iconData[8]  += 0x05; iconData[9]  += 0x1E; iconData[13] += 0x08; break;
        case 7: iconData[8]  += 0x05; iconData[9]  += 0x1E;
                iconData[11] += 0x02; iconData[13] += 0x08; break;
        default: break;
    }
}

// -------------------------------------------------------
// Vent mode (0=Feet, 1=Feet+Demist, 2=Face, 3=Face+Feet)
// -------------------------------------------------------
void LCD_SetVentMode(uint8_t mode)
{
    iconData[12] += 0x01; // Seated Man — always on
    switch (mode)
    {
        case 0: iconData[13] += 0x04; break;  // VENT_FACE — face arrow only
        case 1: iconData[12] += 0x04; break; // VENT_FEET_DEMIST — feet + demist
        case 2: iconData[12] += 0x04; iconData[13] += 0x04; break;  // VENT_FEET_FACE — feet + face
        case 3: iconData[12] += 0x04; break; // VENT_FEET — feet arrow only
        default: break;
    }
}

void LCD_SetCircMode(uint8_t circ){
	iconData[7] += 0x01; //Car ICON
	switch (circ){
		case 0: iconData[7] += 0x04; break; //Fresh air
		case 1: iconData[7] += 0x08; break; //Recirc
	}

}


void LCD_SetAuto(uint8_t Auto){
	if (Auto){
		iconData[7] += 0x10;
	}
	else{
		iconData[7] += 0x01;
	}
}

void LCD_SetAC(uint8_t AC){
	if (AC){
		iconData[8] += 0x08;
	}
}

void LCD_SetECO(uint8_t eco){
	if (eco){
		iconData[8] += 0x10;
	}

}

void LCD_SetDemistFront(uint8_t demist){
	if (demist){
		iconData[12] += 0x02;
	}

}
// -------------------------------------------------------
// Dividers (0=L/H vertical, 1=R/H vertical, 2=HH:MM)
// -------------------------------------------------------
void LCD_SetDivider()
{
    iconData[1]  += 0x10; //Left
    iconData[11] += 0x08; //Right
    iconData[3]  += 0x04; //Time
}

// -------------------------------------------------------
// Minute digit segments (digit=1 for Mm, digit=2 for mM)
// -------------------------------------------------------
void LCD_SetMinDigit(uint8_t digit, uint8_t value)
{
    if (digit == 1)   // Mm units
    {
        // ← no clearing here
        switch (value)
        {
            case 0: iconData[0]+=0x18; iconData[2]+=0x07; iconData[3]+=0x08; break;
            case 1: iconData[0]+=0x08; iconData[2]+=0x04; break;
            case 2: iconData[0]+=0x10; iconData[2]+=0x0E; iconData[3]+=0x08; break;
            case 3: iconData[0]+=0x18; iconData[2]+=0x0E; break;
            case 4: iconData[0]+=0x08; iconData[2]+=0x0D; break;
            case 5: iconData[0]+=0x18; iconData[2]+=0x0B; break;
            default: break;
        }
    }
    else if (digit == 2)  // mM tens
    {
    	switch (value)
    	    {
    	        case 0: iconData[0]+=0x03; iconData[1]+=0x07; iconData[2]+=0x10; break;  // ← add this
    	        case 1: iconData[0]+=0x01; iconData[1]+=0x04; break;
    	        case 2: iconData[0]+=0x02; iconData[1]+=0x0E; iconData[2]+=0x10; break;
    	        case 3: iconData[0]+=0x03; iconData[1]+=0x0E; break;
    	        case 4: iconData[0]+=0x01; iconData[1]+=0x0D; break;
    	        case 5: iconData[0]+=0x03; iconData[1]+=0x0B; break;
    	        case 6: iconData[0]+=0x03; iconData[1]+=0x0B; iconData[2]+=0x10; break;
    	        case 7: iconData[0]+=0x01; iconData[1]+=0x06; break;
    	        case 8: iconData[0]+=0x03; iconData[1]+=0x0F; iconData[2]+=0x10; break;
    	        case 9: iconData[0]+=0x03; iconData[1]+=0x0F; break;
    	        default: break;
    	    }
    	}
}

// -------------------------------------------------------
// Seven segment number (0–9) for temperature/time display
// -------------------------------------------------------
void LCD_SetSevenSegment(uint8_t col, uint8_t value)
{
    switch (value)
    {
        case 0: sevenSegData[0]+=col; sevenSegData[1]+=col; sevenSegData[2]+=col;
                sevenSegData[4]+=col; sevenSegData[5]+=col; sevenSegData[6]+=col; break;
        case 1: sevenSegData[2]+=col; sevenSegData[5]+=col; break;
        case 2: sevenSegData[0]+=col; sevenSegData[2]+=col; sevenSegData[3]+=col;
                sevenSegData[4]+=col; sevenSegData[6]+=col; break;
        case 3: sevenSegData[0]+=col; sevenSegData[2]+=col; sevenSegData[3]+=col;
                sevenSegData[5]+=col; sevenSegData[6]+=col; break;
        case 4: sevenSegData[1]+=col; sevenSegData[2]+=col;
                sevenSegData[3]+=col; sevenSegData[5]+=col; break;
        case 5: sevenSegData[0]+=col; sevenSegData[1]+=col; sevenSegData[3]+=col;
                sevenSegData[5]+=col; sevenSegData[6]+=col; break;
        case 6: sevenSegData[0]+=col; sevenSegData[1]+=col; sevenSegData[3]+=col;
                sevenSegData[4]+=col; sevenSegData[5]+=col; sevenSegData[6]+=col; break;
        case 7: sevenSegData[0]+=col; sevenSegData[2]+=col; sevenSegData[5]+=col; break;
        case 8: sevenSegData[0]+=col; sevenSegData[1]+=col; sevenSegData[2]+=col;
                sevenSegData[3]+=col; sevenSegData[4]+=col; sevenSegData[5]+=col;
                sevenSegData[6]+=col; break;
        case 9: sevenSegData[0]+=col; sevenSegData[1]+=col; sevenSegData[2]+=col;
                sevenSegData[3]+=col; sevenSegData[5]+=col; break;
        case 10:sevenSegData[0]=0xFF; sevenSegData[1]=0xFF; sevenSegData[2]=0xFF;
        		sevenSegData[3]=0xFF; sevenSegData[4]=0xFF; sevenSegData[5]=0xFF;
        		sevenSegData[6]=0xFF; break;
        default: break;
    }
    //LCD_FlushSevenSegment();
}

static void clearHourColumns(void)
{
    for (uint8_t i = 0; i < 7; i++)
    {
        sevenSegData[i] &= ~0x08;  // clear hH column
        sevenSegData[i] &= ~0x10;  // clear Hh column
    }
}

static void clearTempColumns(void)
{
    for (uint8_t i = 0; i < 7; i++)
    {
        sevenSegData[i] &= ~0x01;  // clear temp digit 1
        sevenSegData[i] &= ~0x04;  // clear temp digit 2
        sevenSegData[i] &= ~0x02;  // clear temp digit 3
    }
}

// -------------------------------------------------------
// Set minute digit
// col 0 = Mm (units/right), col 1 = mM (tens/left)
// value 0-9
// -------------------------------------------------------
void LCD_SetMinute(uint8_t col, uint8_t value)
{
    LCD_SetMinDigit(col == 0 ? 1 : 2, value);
    //LCD_FlushIcons();
}

// -------------------------------------------------------
// Set hour digit
// col 0 = hH (units), col 1 = Hh (tens)
// value 0-9
// -------------------------------------------------------
void LCD_SetHour(uint8_t col, uint8_t value)
{
    // col 0 = hH units (0x08), col 1 = Hh tens (0x10)
    // Don't clear sevenSegData — both digits share the same array
    // Just set the bits for this column
    LCD_SetSevenSegment(col == 0 ? 0x08 : 0x10, value);
}

void LCD_SetTempDigit(uint8_t col, uint8_t value)
{
    static const uint8_t tempCols[3] = {0x01, 0x04, 0x02};
    if (col > 2) return;
    LCD_SetSevenSegment(tempCols[col], value);
}

void LCD_SetTime()
{
    clearHourColumns();              // ← only clears hour bits
    Time_GetTime();

    if (sTime.Minutes == lastMinute)
    	if (!climateReceived){
    		return;  // nothing to do — minute bits already preserved in iconData
    	}

    lastMinute = sTime.Minutes;

    iconData[0] = 0x00;
    iconData[1] = 0x00;
    iconData[2] = 0x00;
    iconData[3] = 0x00;

    clearHourColumns();
    LCD_SetHour(1, sTime.Hours / 10);
    LCD_SetHour(0, sTime.Hours % 10);
    LCD_FlushSevenSegment();

	LCD_SetMinute(0, sTime.Minutes / 10);
	LCD_SetMinute(1, sTime.Minutes % 10);
	//LCD_FlushIcons();
}

void LCD_SetTemperature(uint8_t tens, uint8_t units, uint8_t decimal)
{
    //uint8_t integer = (uint8_t)temp;
    //uint8_t decimal = (uint8_t)((temp - integer) * 10 + 0.5f);

    clearTempColumns();              // ← only clears temp bits
    LCD_SetTempDigit(0, tens);
    LCD_SetTempDigit(1, units);
    LCD_SetTempDigit(2, decimal);
    LCD_FlushSevenSegment();

    iconData[13] |= 0x02;  // decimal point between temp and decimal
    iconData[6]  |= 0x04;  // thermo symbol
    iconData[6]  |= 0x10;  // deg C
    //LCD_FlushIcons();
}

void LCD_Test(void)
{
    LCD_Print("LCD TEST    ");
    HAL_Delay(1000);

    #define ICON_TEST(label, reg, bits) \
        LCD_Print(label); \
        iconData[reg] |= bits; \
        LCD_FlushIcons(); \
        HAL_Delay(1500); \
        LCD_ResetIcons();

    ICON_TEST("mM R/H low  ", 0, 0x01)
    ICON_TEST("mM lower    ", 0, 0x02)
    ICON_TEST("Min fullstop", 0, 0x04)
    ICON_TEST("Mm R/H lower", 0, 0x08)
    ICON_TEST("Mn lower    ", 0, 0x10)
    ICON_TEST("mM L/H upper", 1, 0x01)
    ICON_TEST("mM Upper    ", 1, 0x02)
    ICON_TEST("mM R/H upper", 1, 0x04)
    ICON_TEST("mM centre   ", 1, 0x08)
    ICON_TEST("L/H Divider ", 1, 0x10)
    ICON_TEST("Mm L/H upper", 2, 0x01)
    ICON_TEST("Mn upper    ", 2, 0x02)
    ICON_TEST("Mm R/H upper", 2, 0x04)
    ICON_TEST("Mm centre   ", 2, 0x08)
    ICON_TEST("mM L/H lower", 2, 0x10)
    ICON_TEST("HH:MM Div   ", 3, 0x04)
    ICON_TEST("Mm L/H lower", 3, 0x08)
    ICON_TEST("CD IN       ", 3, 0x10)
    ICON_TEST("MD IN       ", 4, 0x10)
    ICON_TEST("Dolby Logo  ", 5, 0x01)
    ICON_TEST("Stereo ST   ", 5, 0x10)
    ICON_TEST("AF          ", 6, 0x01)
    ICON_TEST("AMB         ", 6, 0x02)
    ICON_TEST("Thermo      ", 6, 0x04)
    ICON_TEST("Deg F       ", 6, 0x08)
    ICON_TEST("Deg C       ", 6, 0x10)
    ICON_TEST("AC Car Logo ", 7, 0x01)
    ICON_TEST("PTY         ", 7, 0x02)
    ICON_TEST("Fresh Air   ", 7, 0x04)
    ICON_TEST("Recirc Air  ", 7, 0x08)
    ICON_TEST("AUTO        ", 7, 0x10)
    ICON_TEST("Fan 4       ", 8, 0x01)
    ICON_TEST("RPT         ", 8, 0x02)
    ICON_TEST("Fan 5       ", 8, 0x04)
    ICON_TEST("A/C         ", 8, 0x08)
    ICON_TEST("ECO         ", 8, 0x10)
    ICON_TEST("Fan Base    ", 9, 0x02)
    ICON_TEST("Fan 1       ", 9, 0x04)
    ICON_TEST("Fan 2       ", 9, 0x08)
    ICON_TEST("Fan 3       ", 9, 0x10)
	ICON_TEST("Fan 4       ", 8, 0x01)
    ICON_TEST("Fan 5       ", 8, 0x04)
    ICON_TEST("Fan 6       ", 13, 0x08)
	ICON_TEST("Fan 7       ", 11, 0x02)
    ICON_TEST("Auto-M      ", 10, 0x01)
    ICON_TEST("TP          ", 10, 0x02)
    ICON_TEST("TA          ", 10, 0x04)
    ICON_TEST("RDM         ", 10, 0x08)
    ICON_TEST("DDRAM #10.  ", 10, 0x10)
    ICON_TEST("Fan 7       ", 11, 0x02)
    ICON_TEST("Min/Sec Mark", 11, 0x04)
    ICON_TEST("R/H Divider ", 11, 0x08)
    ICON_TEST("DDRAM #11.  ", 11, 0x10)
    ICON_TEST("Seated Man  ", 12, 0x01)
    ICON_TEST("Front Demist", 12, 0x02)
    ICON_TEST("Feet Arrow  ", 12, 0x04)
    ICON_TEST("Temp Dec    ", 13, 0x02)
    ICON_TEST("Face Arrow  ", 13, 0x04)
    ICON_TEST("Fan 6       ", 13, 0x08)
    ICON_TEST("Mid Colon   ", 13, 0x10)

    // Fan speed sequence
    LCD_Print("Fan Test    ");
    for (uint8_t f = 1; f <= 7; f++){
        LCD_SetFanSpeed(f);
        LCD_FlushIcons();
        HAL_Delay(500);
        LCD_ResetIcons();
    }

    // Minutes Mm
    LCD_Print("Mm test     ");
    for (uint8_t m = 0; m <= 5; m++){
        LCD_SetMinDigit(1, m);
        LCD_FlushIcons();
        HAL_Delay(500);
        LCD_ResetIcons();
    }

    // Minutes mM
    LCD_Print("mM test     ");
    for (uint8_t m = 0; m <= 9; m++){
        LCD_SetMinDigit(2, m);
        LCD_FlushIcons();
        HAL_Delay(500);
        LCD_ResetIcons();
    }

    // hH test (units of hours)
    LCD_Print("Testing hH  ");
    for (uint8_t j = 0; j < 10; j++){
        LCD_SetSevenSegment(0x08, j);
        LCD_FlushSevenSegment();
        HAL_Delay(300);
    }

    // Hh test (tens of hours)
    LCD_Print("Testing Hh  ");
    for (uint8_t j = 0; j < 10; j++){
        LCD_SetSevenSegment(0x10, j);
        LCD_FlushSevenSegment();
        HAL_Delay(300);
    }

    // Temperature digits — matches Arduino tempOrder = {0x01, 0x04, 0x02}
    uint8_t tempOrder[3] = {0x01, 0x04, 0x02};
    const char* tempLabels[3] = {"Temp Dig 1  ", "Temp Dig 2  ", "Temp Dig 3  "};
    for (uint8_t i = 0; i < 3; i++){
        LCD_Print(tempLabels[i]);
        for (uint8_t j = 0; j < 10; j++){
            LCD_SetSevenSegment(tempOrder[i], j);
            LCD_FlushSevenSegment();
            HAL_Delay(300);
        }
    }

    LCD_Print("TEST DONE   ");
    HAL_Delay(1000);



    #undef ICON_TEST
}
