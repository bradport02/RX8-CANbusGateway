#include "protocol.h"
#include "Adafruit_NeoPixel.h"

bool waitForACK = false;
uint32_t ackTimeout = 0;
uint8_t lastACKCmd = 0;

uint8_t Protocol_CRC8(uint8_t *data, uint8_t len) {
    uint8_t crc = 0x00;
    for (uint8_t i = 0; i < len; i++) {
        crc ^= data[i];
        for (uint8_t j = 0; j < 8; j++) {
            if (crc & 0x80) {
                crc = (crc << 1) ^ 0x07;
            } else {
                crc <<= 1;
            }
        }
    }
    return crc;
}

// NOTE: ESP32 builds the 12-byte packet to send TO the STM32
void Protocol_BuildTx(EspPacket_t *pkt, uint8_t start, uint8_t cmd, uint8_t *data, uint8_t len) {
    memset(pkt, PADDING, sizeof(EspPacket_t));
    pkt->start = start;
    pkt->cmd = cmd;
    pkt->len = len;

    if (data != NULL && len > 0) {
        memcpy(pkt->data, data, len > RX_DATA_LEN ? RX_DATA_LEN : len);
    }

    // CRC is calculated on CMD, LEN, and DATA
    uint8_t crc_buf[RX_DATA_LEN + 2];
    crc_buf[0] = pkt->cmd;
    crc_buf[1] = pkt->len;
    memcpy(&crc_buf[2], pkt->data, RX_DATA_LEN);
    pkt->crc = Protocol_CRC8(crc_buf, RX_DATA_LEN + 2);
}

void Protocol_Send(HardwareSerial *serial, uint8_t start, uint8_t cmd, uint8_t *data, uint8_t len) {
    EspPacket_t pkt;
    Protocol_BuildTx(&pkt, start, cmd, data, len);
    
    // Replace HAL_UART_Transmit with serial->write
    serial->write((uint8_t*)&pkt, ESP_PACKET_LEN);
}

void Protocol_SendACK(HardwareSerial *serial, uint8_t start, uint8_t cmd) {
    uint8_t ack = ACK; // Note: In your spreadsheet ACK data is 0xF0, adjust if needed!
    Serial.println("Sending ACK");
    Protocol_Send(serial, start, cmd, &ack, 1);
}

void Protocol_SendNACK(HardwareSerial *serial, uint8_t start, uint8_t cmd) {
    uint8_t nack = NACK; // Spreadsheet says NACK is 0xF1, adjust if needed!
    Protocol_Send(serial, start, cmd, &nack, 1);
}

void Protocol_ProcessPacket(HardwareSerial *serial, uint8_t *buf) {
    // ESP32 casts the incoming buffer to the 20-byte STM32 packet
    StmPacket_t *pkt = (StmPacket_t*)buf;

    if (pkt->start != STM) {
        Protocol_SendNACK(serial, ESP, 0x00);
        return;
    }

    // Check incoming 20-byte packet CRC
    uint8_t crc_buf[TX_DATA_LEN + 2];
    crc_buf[0] = pkt->cmd;
    crc_buf[1] = pkt->len;
    memcpy(&crc_buf[2], pkt->data, TX_DATA_LEN);
    uint8_t expected = Protocol_CRC8(crc_buf, TX_DATA_LEN + 2);

    if (expected != pkt->crc) {
        Protocol_SendNACK(serial, ESP, pkt->cmd);
        return;
    }

    Protocol_HandlePacket(serial, pkt);
}

void Protocol_HandlePacket(HardwareSerial *serial, StmPacket_t *pkt) {
    switch (pkt->cmd) {
        case CMD_HANDSHAKE:
            Serial.println("Handshake Received");
            Protocol_SendACK(serial, ESP, CMD_HANDSHAKE);
            //Protocol_Send(serial, ESP, CMD_HANDSHAKE, NULL, 0);
            break;

        case CMD_LCD:
            if (pkt->data[0] == ACK){
                lastACKCmd = CMD_LCD;
            }
            break;

        case CMD_TEMP_UP:
            if (pkt->data[0] == ACK){
                lastACKCmd = CMD_TEMP_UP;
            }
            break;
        
        case CMD_TEMP_DOWN:
            if (pkt->data[0] == ACK){
                lastACKCmd = CMD_TEMP_DOWN;
            }
            break;

        case CMD_FAN_UP:
            if (pkt->data[0] == ACK){
                lastACKCmd = CMD_FAN_UP;
            }
            break;
        
        case CMD_FAN_DOWN:
            if (pkt->data[0] == ACK){
                lastACKCmd = CMD_FAN_DOWN;
            }
            break;

        case CMD_VENT_MODE:
            if (pkt->data[0] == ACK){
                lastACKCmd = CMD_VENT_MODE;
            }
            break;
        default:
            Protocol_SendNACK(serial, ESP, pkt->cmd);
            break;
    }
}

void Protocol_PollSerial(HardwareSerial *serial) {
    // Static variables keep their values between function calls
    static uint8_t rxBuf[STM_PACKET_LEN];
    static uint8_t rxIndex = 0;

    // Process every byte currently sitting in the mailbox
    while (serial->available() > 0) {
        uint8_t incomingByte = serial->read();
        if (rxIndex == 0 && incomingByte != STM) {
            //ignore it and stay at index 0.
            continue; 
        }

        rxBuf[rxIndex] = incomingByte;
        rxIndex++;

        //CHeck if all 20 bytes have arrived
        if (rxIndex == STM_PACKET_LEN) {
            //Serial.println("--- Full 20-Byte Packet Assembled! ---");
            Protocol_ProcessPacket(serial, rxBuf);
            rxIndex = 0; 
        }
    }
}


void Protocol_WaitForACK(uint8_t cmd){
    uint32_t start = millis();
    while (millis() - start < 500)   // 500ms timeout
    {
        Protocol_PollSerial(&Serial1);
        if (lastACKCmd == cmd)
        {
            lastACKCmd = 0;
            break;
        }
    }
}