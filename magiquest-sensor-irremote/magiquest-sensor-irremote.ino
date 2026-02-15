#include <IRremote.hpp>

// MagiQuest protocol constants
#define MAGIQUEST_PERIOD     1150
#define MAGIQUEST_TOLERANCE  200

union magiquest {
  uint64_t llword;
  uint8_t  byte[8];
  uint32_t lword[2];
  struct {
    uint16_t magnitude;
    uint32_t wand_id;
    uint8_t  padding;
    uint8_t  scrap;
  } cmd;
};

#define ERR 0
#define DECODED 1

int recv_pin = A0;
magiquest data;

bool customMatchMark(uint16_t measured, uint16_t desired);
int32_t decodeMagiQuest(magiquest *mdata);

void setup() {
  Serial.begin(115200);
  while (!Serial) { ; }
  
  Serial.println("=== MagiQuest IR Decoder ===");
  Serial.println("Point your wand at the sensor and activate it");
  IrReceiver.begin(recv_pin, ENABLE_LED_FEEDBACK);
}

void loop() {
  if (IrReceiver.decode()) {
    Serial.println("\n--- IR Signal Received ---");
    Serial.print("Protocol: ");
    Serial.println(getProtocolString(IrReceiver.decodedIRData.protocol));
    Serial.print("Address: 0x");
    Serial.println(IrReceiver.decodedIRData.address, HEX);
    Serial.print("Command: 0x");
    Serial.println(IrReceiver.decodedIRData.command, HEX);
    Serial.print("Raw Data: 0x");
    Serial.println(IrReceiver.decodedIRData.decodedRawData, HEX);
    
    if (decodeMagiQuest(&data) == DECODED) {
      Serial.println("*** MagiQuest Decoded! ***");
      Serial.print("Wand ID: ");
      Serial.println(data.cmd.wand_id, HEX);
      Serial.print("Magnitude: ");
      Serial.println(data.cmd.magnitude, HEX);
      Serial.print("Raw as struct - Wand: ");
      Serial.print(data.cmd.wand_id);
      Serial.print(", Mag: ");
      Serial.println(data.cmd.magnitude);
    } else {
      Serial.println("Custom decoder failed");
    }
    
    Serial.println("--- End Signal ---\n");
    IrReceiver.resume();
  }
  delay(100);
}

bool customMatchMark(uint16_t measured, uint16_t desired) {
  return (measured >= desired - MAGIQUEST_TOLERANCE && 
          measured <= desired + MAGIQUEST_TOLERANCE);
}

int32_t decodeMagiQuest(magiquest *mdata) {
  // Check if MagiQuest protocol was detected
  if (IrReceiver.decodedIRData.protocol != MAGIQUEST) {
    return ERR;
  }
  
  // Clear the data structure first
  mdata->llword = 0;
  
  // Extract data from the IRremote library's decoded result
  // Address contains wand_id, Command contains magnitude
  mdata->cmd.wand_id = IrReceiver.decodedIRData.address;
  mdata->cmd.magnitude = IrReceiver.decodedIRData.command;
  
  // Store the complete raw decoded data as well
  uint64_t rawData = IrReceiver.decodedIRData.decodedRawData;
  
  // For debugging - show what we extracted
  Serial.print("Extracted - Wand ID: 0x");
  Serial.print(mdata->cmd.wand_id, HEX);
  Serial.print(" (");
  Serial.print(mdata->cmd.wand_id);
  Serial.print("), Magnitude: 0x");
  Serial.print(mdata->cmd.magnitude, HEX);
  Serial.print(" (");
  Serial.print(mdata->cmd.magnitude);
  Serial.println(")");
  
  // Validation - accept any non-zero wand_id
  if (mdata->cmd.wand_id != 0) {
    return DECODED;
  }
  
  return ERR;
}
