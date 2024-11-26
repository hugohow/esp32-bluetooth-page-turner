// "2a:07:98:02:8a:a2"

#include <Arduino.h>
#include <ArduinoBLE.h>

void ProcessHIDReport(uint8_t *ucData, int iLen);

enum
{
  CONTROLLER_ACGAM_R1 = 0,
  CONTROLLER_MINIPLUS,
  CONTROLLER_UNKNOWN
};

static int iControllerType;
volatile static int bConnected, bChanged;

// Current button state
// The first 8 bits are for the (up to 8) push buttons
static uint16_t u16Buttons;
// bits to define the direction pad since it's not analog
#define BUTTON_LEFT 0x100
#define BUTTON_RIGHT 0x200
#define BUTTON_UP 0x400
#define BUTTON_DOWN 0x800

// #define ENABLE_1 12
// #define ENABLE_2 14
#define INPUT_1 27
#define INPUT_2 26
#define INPUT_3 14
#define INPUT_4 12

// // Graphics for the controller real time display
// const uint8_t ucArrow[] PROGMEM = {
//     0x01, 0x80, 0x02, 0x40, 0x04, 0x20, 0x08, 0x10, 0x10, 0x08, 0x20, 0x04, 0x40, 0x02, 0x80, 0x01,
//     0x80, 0x01, 0xfc, 0x3f, 0x04, 0x20, 0x04, 0x20, 0x04, 0x20, 0x04, 0x20, 0x04, 0x20, 0x07, 0xe0};

// const uint8_t ucFilledArrow[] PROGMEM = {
//     0x01, 0x80, 0x03, 0xc0, 0x07, 0xe0, 0x0f, 0xf0, 0x1f, 0xf8, 0x3f, 0xfc, 0x7f, 0xfe, 0xff, 0xff,
//     0xff, 0xff, 0xff, 0xff, 0x07, 0xe0, 0x07, 0xe0, 0x07, 0xe0, 0x07, 0xe0, 0x07, 0xe0, 0x07, 0xe0};

// const uint8_t ucSquare[] PROGMEM = {
//     0x00, 0x00, 0x00, 0x00, 0x3f, 0xfc, 0x20, 0x04, 0x20, 0x04, 0x20, 0x04, 0x20, 0x04, 0x20, 0x04,
//     0x20, 0x04, 0x20, 0x04, 0x20, 0x04, 0x20, 0x04, 0x20, 0x04, 0x3f, 0xfc, 0x00, 0x00, 0x00, 0x00};

// const uint8_t ucFilledSquare[] PROGMEM = {
//     0x00, 0x00, 0x00, 0x00, 0x3f, 0xfc, 0x3f, 0xfc, 0x3f, 0xfc, 0x3f, 0xfc, 0x3f, 0xfc, 0x3f, 0xfc,
//     0x3f, 0xfc, 0x3f, 0xfc, 0x3f, 0xfc, 0x3f, 0xfc, 0x3f, 0xfc, 0x3f, 0xfc, 0x00, 0x00, 0x00, 0x00};

void setup()
{
  Serial.begin(115200);

  // pinMode(ENABLE_1, OUTPUT);
  // pinMode(ENABLE_2, OUTPUT);
  pinMode(INPUT_1, OUTPUT);
  pinMode(INPUT_2, OUTPUT);
  pinMode(INPUT_3, OUTPUT);
  pinMode(INPUT_4, OUTPUT);
  while (!Serial)
    ;

  Serial.println("Scanning for BLE ctrl");

  if (!BLE.begin())
  {
    Serial.println("starting BLE failed!");
    while (1)
      ;
  }
  BLE.scan();

  u16Buttons = 0;
}

//
// Callback which handles HID report updates
//
void HIDReportWritten(BLEDevice central, BLECharacteristic characteristic)
{
  int iLen, i;
  uint8_t ucTemp[128];

  // central wrote new HID report info
  iLen = characteristic.readValue(ucTemp, sizeof(ucTemp));
  ProcessHIDReport(ucTemp, iLen);
}

void monitorActions(BLEDevice peripheral)
{

  char ucTemp[128];
  int i, iLen, iCount;
  // BLECharacteristic hidRep[16];
  BLEService hidService;
  // int ihidCount = 0;

  // connect to the peripheral
  Serial.println("Connecting...");
  if (peripheral.connect())
  {
    Serial.println("Connected");
  }
  else
  {
    Serial.println("Failed to connect!");
    return;
  }

  // discover peripheral attributes
  //  Serial.println("Discovering service 0x1812 ...");
  if (peripheral.discoverService("1812"))
  {
    Serial.println("0x1812 discovered");
  }
  else
  {
    Serial.println("0x1812 disc failed");
    peripheral.disconnect();

    while (1)
      ;
    return;
  }

  hidService = peripheral.service("1812"); // get the HID service
  iCount = hidService.characteristicCount();
  for (i = 0; i < iCount; i++)
  {
    BLECharacteristic bc = hidService.characteristic(i);
    if (strcasecmp(bc.uuid(), "2a4D") == 0) // enable notify
    {
      //      hidRep[ihidCount++] = bc;
      bc.subscribe();
      bc.setEventHandler(BLEWritten, HIDReportWritten);
    }
  }

  BLECharacteristic protmodChar = hidService.characteristic("2A4E"); // get protocol mode characteristic
  if (protmodChar != NULL)
  {
    protmodChar.writeValue((uint8_t)0x01); // set protocol report mode (we want reports)
  }

  // DEBUG - for now, we're not parsing the report map
  //  BLECharacteristic reportmapChar = hidService.characteristic("2A4B");
  //  if (reportmapChar != NULL)
  //  {
  //    iLen = reportmapChar.readValue(ucTemp, 128);
  //    Serial.print("Read report map: ");
  //    for (i=0; i<iLen; i++)
  //    {
  //      Serial.print("0x"); Serial.print(ucTemp[i], HEX); Serial.print(", ");
  //    }
  //    Serial.println(" ");
  //  }

  // subscribe to the HID report characteristic
  //  Serial.println("Subscribing to HID report characteristic(s) ...");
  //  if (!ihidCount) {
  //    Serial.println("no HID report characteristics found!", 0);
  //    peripheral.disconnect();
  //    return;
  //  }
  //  else
  //  {
  //    for (i=0; i<ihidCount; i++) // The reports are split among multiple characteristics (different buttons/controls)
  //    {
  //      hidRep[i].subscribe(); // subscribe to all of them
  //      hidRep[i].setEventHandler(BLEWritten, HIDReportWritten);
  //    }
  //  }
  Serial.println("Ready to use!");
  delay(2000);
  bChanged = 1; // force a repaint before the controls are touched, otherwise it will look like it's hung

  while (peripheral.connected())
  {
    // while the peripheral is connected
    if (bChanged)
    {
      bChanged = 0;
    }
  } // while connected
}

void loop()
{
  char szTemp[64];

  bConnected = 0;
  // check if a peripheral has been discovered
  BLEDevice peripheral = BLE.available();

  // digitalWrite(ENABLE_1, HIGH);
  // digitalWrite(ENABLE_2, HIGH);

  if (peripheral)
  {
    // Check if the peripheral is a HID device
    if (peripheral.advertisedServiceUuid() == "1812")
    {
      // stop scanning
      BLE.stopScan();

      monitorActions(peripheral);

      // peripheral disconnected, start scanning again
      BLE.scan();
    }
  }
}

void goRight()
{
  digitalWrite(INPUT_1, LOW);
  digitalWrite(INPUT_2, HIGH);
  digitalWrite(INPUT_3, LOW);
  digitalWrite(INPUT_4, HIGH);
  delay(1000);
}

void goLeft()
{
  digitalWrite(INPUT_1, HIGH);
  digitalWrite(INPUT_2, LOW);
  digitalWrite(INPUT_3, HIGH);
  digitalWrite(INPUT_4, LOW);
  delay(1000);
}

void goUp()
{
  digitalWrite(INPUT_1, HIGH);
  digitalWrite(INPUT_2, LOW);
  digitalWrite(INPUT_3, LOW);
  digitalWrite(INPUT_4, HIGH);
  delay(1000);
}

void goDown()
{
  digitalWrite(INPUT_1, LOW);
  digitalWrite(INPUT_2, HIGH);
  digitalWrite(INPUT_3, HIGH);
  digitalWrite(INPUT_4, LOW);
  delay(1000);
}

void stop()
{
  digitalWrite(INPUT_1, LOW);
  digitalWrite(INPUT_2, LOW);
  digitalWrite(INPUT_3, LOW);
  digitalWrite(INPUT_4, LOW);
}

//
// Convert HID report bytes into
// my button and joystick variables
//
void ProcessHIDReport(uint8_t *ucData, int iLen)
{
  int i;

  bChanged = 1;

  // Serial.println("Unknown controller type");
  // ucData
  // iLen
  // for (int i = 0; i < iLen; i++)
  // {
  //   Serial.print(ucData[0], BIN);
  // }
  // Serial.printf("%02x, %02x, %02x\n", ucData[0], ucData[1], ucData[2]);

  // define u16Buttons

  // 01 f8 quand on appuie sur le premier button

  // first click ?
  // 01, f8, 7f
  // a0, 30, e8
  // 00, 00, 00
  // 00, 00, 00

  // haut
  // 01, 18, 80
  // 3c, 80, 0c
  // 01, 00, 00
  // 02, 30, 00
  // 00, 00, 05
  // 00, 00, 05
  // 00, 00, 05
  // 00, 00, 00
  // 01, 18, 80
  // 3c, 80, 0c

  // keep pressing haut
  // e9, 00, 00
  // 00, 00, 00

  // bas
  // 01, f8, 7f
  // 3c, 40, ec
  // 01, 00, 00
  // 02, d0, ff
  // 00, e0, fc
  // 00, e0, fc
  // 00, e0, fc
  // 00, 00, 00
  // 01, 18, 80
  // 3c, 80, 0c

  // keep pressing bas
  // ea, 00, 00
  // 00, 00, 00

  // gauche
  // 01, 18, 80
  // 28, 80, 11

  // droite
  // ff, 17, 80
  // ae, 8f, 11

  // active haut
  // 01, 18, 80
  // 3c, 80, 0c

  // active bas
  // 01, f8, 7f
  // 3c, 40, ec

  if (ucData[0] == 0x28 && ucData[1] == 0x80 && ucData[2] == 0x11)
  {
    Serial.print("gauche\n");
    goRight();
  }
  else if (ucData[0] == 0xae && ucData[1] == 0x8f && ucData[2] == 0x11)
  {
    Serial.print("droite\n");
    goLeft();
  }
  else if (ucData[0] == 0xea && ucData[1] == 0x00 && ucData[2] == 0x00)
  {
    goDown();
    Serial.print("keep bas\n");
  }
  else if (ucData[0] == 0xe9 && ucData[1] == 0x00 && ucData[2] == 0x00)
  {
    goUp();
    Serial.print("keep haut\n");
  }
  else if (ucData[0] == 0x02 && ucData[1] == 0x30 && ucData[2] == 0x00)
  {
    goUp();
    Serial.print("haut\n");
  }
  else if (ucData[0] == 0x02 && ucData[1] == 0xd0 && ucData[2] == 0xff)
  {
    goDown();
    Serial.print("bas\n");
  }
  else if (ucData[0] == 0x3c && ucData[1] == 0x40 && ucData[2] == 0xec)
  {
    goDown();
    Serial.print("bas\n");
  }

  // center
  // 01, f8, 7f
  // a0, 30, e8
  // 01, 00, 00
  // 00, 00, 00
  // 00, 00, 00
  // 00, 00, 00
  // 00, 00, 00

  // heart

  // 01, f8, 7f
  // 00, 00, 00
  // 01, 00, 00
  // 00, 00, 00
  // 01, f8, 7f
  // 00, 00, 00
  // 01, 00, 00
  // 00, 00, 00

  // type, valeur, valeur
  // ça finit par 0c
  // mettre toutes les trams par paquet dans chat gpt.
  // ça commence par 01 et finit par 0c

  // bas
  // 01, f8, 7f
  // 3c, 40, ec
  // 01, 00, 00
  // 02, d0, ff
  // 00, e0, fc
  // 00, e0, fc
  // 00, e0, fc
  // 00, 00, 00
  // 01, 18, 80
  // 3c, 80, 0c

  // gauche
  // 01, 18, 80
  // 28, 80, 11
  // 01, 00, 00
  // 96, 00, 00
  // 96, 00, 00
  // 96, 00, 00
  // 00, 00, 00
  // 00, 00, 00
  // 01, 18, 80
  // 3c, 80, 0c

  // RIGHT
  // AE8F11
  // if (ucData[0] == 0xAE && ucData[1] == 0x8F && ucData[2] == 0x11)
  // {
  //   goRight();
  //   Serial.print("\nRIGHT\n");
  // }
  // else if (ucData[0] == 0x3C && ucData[1] == 0x40 && ucData[2] == 0xEC)
  // {
  //   goDown();
  //   Serial.print("\nDOWN\n");
  // }
  // else if (ucData[0] == 0x28 && ucData[1] == 0x80 && ucData[2] == 0x11)
  // {
  //   goLeft();
  //   Serial.print("\nLEFT\n");
  // }
  // else if (ucData[0] == 0x3C && ucData[1] == 0x80 && ucData[2] == 0xC)
  // {
  //   goUp();
  //   Serial.print("\nUP\n");
  // }
  // else if (ucData[0] == 0x1F && ucData[1] == 0x87 && ucData[2] == 0xF)
  // {
  //   Serial.print("\nHEART\n");
  // }
  // else if (ucData[0] == 0xA0 && ucData[1] == 0x30 && ucData[2] == 0xE8)
  // {
  //   stop();
  //   Serial.print("\nCENTER\n");
  // }

  stop();
}
