#define EEPROM_I2C_ADDR 0x50  // I2C address of AT24C32N
#define EEPROM_STATUS_ADDR 0x00  // Memory location to store gate status

// Function to write status to AT24C32N EEPROM
void writeEEPROM(uint16_t address, String data) {
    Wire.beginTransmission(EEPROM_I2C_ADDR);
    Wire.write((address >> 8) & 0xFF);  // MSB of address
    Wire.write(address & 0xFF);         // LSB of address

    for (size_t i = 0; i < data.length(); i++) {  // Write string byte by byte
        Wire.write((uint8_t)data[i]);
    }
    
    Wire.write((uint8_t)'\0');  // Null-terminate the string in EEPROM
    Wire.endTransmission();
    delay(10);  // Allow EEPROM to complete write operation
}


// Function to read status from AT24C32N EEPROM
String readEEPROM(uint16_t address, uint8_t length) {
    String data = "";
    Wire.beginTransmission(EEPROM_I2C_ADDR);
    Wire.write((address >> 8) & 0xFF);  // MSB of address
    Wire.write(address & 0xFF);         // LSB of address
    Wire.endTransmission();
    
    Wire.requestFrom(EEPROM_I2C_ADDR, length);
    while (Wire.available()) {
        char c = Wire.read();
        if (c == '\0') break;  // Stop reading at null character
        data += c;
    }
    return data;
}