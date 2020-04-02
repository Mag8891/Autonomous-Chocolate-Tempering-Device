#include <OneWire.h>
#include <DallasTemperature.h>
#include <Servo.h>

#define ONE_WIRE_BUS 2 // Data BUS wire is plugged into digital pin 2 on the Arduino (Contact Sensor)
#define RELAY1 8   //Peltier Chip 1
#define RELAY2 7   //Peltier Chip 2
#define RELAY3 6   //Peltier Chip 3
#define RELAY4 5   //Peltier Chip 4
#define MOTOR 4    //Mixing Motor
#define IRTemp 3     // KY-028 digital interface (IR Temperature Sensor)
#define LED 10

OneWire oneWire(ONE_WIRE_BUS);  // Setup a oneWire instance to communicate with any OneWire device (contact Sensor)
DallasTemperature sensors(&oneWire);  // Pass oneWire reference to DallasTemperature library (Contact Sensor)

int Temp = 0;  // Variable storing contact temperature values in Celcius
int MilkPin = A1;
int DarkPin = A2;
int ResetPin = A3;
int MilkReading = 0;  //Analog Read values for Milk Chocolate Button
int DarkReading = 0;  //Analog Read values for Dark Chocolate Button
int ResetReading = 0; //Analog Read values for Reset Button
int Milk = 0; // 1 and 0 values to determine whether device is tempering milk chocolate
int Dark = 0; // 1 and 0 values to determine whether device is tempering dark chocolate

//Hopper value declarations
unsigned long HopperStartTime; // Save start time for Hopper system
unsigned long HopperCurrentTime; // Timer for Hopper
const unsigned long HopperDelay = 15000; // 15 sec delay for Hopper system
Servo HopperServo; //Declare Servo

//State Variables
int Cooling = 0;
int Tempering = 0;
int End = 0;

  
void setup(void)
{
  sensors.begin();  // Search for connected sensor (Contact Sensor DS18B20) and set 12 bit resolution (0.125C degree increment)
  Serial.begin(9600);
  pinMode(IRTemp, INPUT);  //Declaring input pin for IR Temperature Sensor
  HopperServo.attach(9); //Declare Hopper Servo attachment to Arduino pin 9
}

//---------------Main Loop---------------
void loop(void)
{ 
  ReadButtons();
  ReadCTemp();

  // If Milk Chocolate button pressed, start milk chocolate tempering process. Stop when reset button is pressed.
  if(Milk == 1){
    TemperMilk();

    if(End == 1){
      ResetSystem();
    }
  }

  // If Dark Chocolate button pressed, start dark chocolate tempering process. Stop when reset button is pressed.
  if(Dark == 1){
    TemperDark();

    if(End == 1){
      ResetSystem();
    }
  }

  // When in idle state, all components are off.
  if(Milk == 0 && Dark == 0){
    analogWrite(LED, 255); //LED on when not tempering chocolate
    MotorOff();
    PeltierOff();
  }
  else{
    analogWrite(LED, 0); //LED off when tempering chocolate
  }
}



//---------------------Functions Below ----------------------------
void TemperMilk(void){
// Tempering process: Room Temperature -> 46C -> 27C -> 30C -> END
  MotorOn(); // Start motor
  
  if(Cooling == 0 && Temp < 46){
    PeltierOn();
  }
  else if(Cooling == 0 && Temp >46){
    Cooling = 1; 
  }

  if(Cooling == 1 && Tempering == 0 && Temp > 27){
    PeltierOff();
    HopperOn();
    HopperStartTime = millis();
  }
  else if(Cooling == 1 && Tempering == 0 && Temp < 27){
    Tempering = 1;
  }

  if(Tempering == 1 && End == 0 && Temp < 30){
    PeltierOn();
  }
  else if(Tempering == 1 && End == 0 && Temp > 30){
    PeltierOff();
    End = 1;
  }
  
}

//-----------Tempers dark chocolate---------------
void TemperDark(void){
// Tempering process: Room Temperature -> 49C -> 28C -> 32C -> END
  MotorOn(); // Start motor
  
  if(Cooling == 0 && Temp < 49){
    PeltierOn();
  }
  else if(Cooling == 0 && Temp > 49){
    Cooling = 1; 
  }

  if(Cooling == 1 && Tempering == 0 && Temp > 28){
    PeltierOff();
    HopperOn();
    HopperStartTime = millis();
  }
  else if(Cooling == 1 && Tempering == 0 && Temp < 28){
    Tempering = 1;
  }

  if(Tempering == 1 && End == 0 && Temp < 32){
    PeltierOn();
  }
  else if(Tempering == 1 && End == 0 && Temp > 32){
    PeltierOff();
    End = 1;
  }
  
}

//------------Reset------------------
void ResetSystem(void){
    Milk = 0;
    Dark = 0;
    Cooling = 0;
    Tempering = 0;
    End = 0;
}

//-----------Read Milk, Dark, Reset buttons--------------
void ReadButtons(void){
  MilkReading = analogRead(MilkPin);
  DarkReading = analogRead(DarkPin);
  ResetReading = analogRead(ResetPin);
  
  if(ResetReading>500){
    ResetSystem();
  }
  
  if(MilkReading>500){
    Milk = 1;
  }

  if(DarkReading>500){
    Dark = 1;
  }
}

//-----------Read Contact Sensor-----------
void ReadCTemp(void){
  sensors.requestTemperatures(); // Send the command to get temperatures
  Temp = sensors.getTempCByIndex(0); //Store Temperature in Celcius
}


//-----------Hopper Servo Controls---------------
void HopperOn(void){
  HopperCurrentTime = millis();
  if(HopperCurrentTime < HopperStartTime){
     HopperServo.write(0);
  }
  if(HopperStartTime > HopperStartTime + HopperDelay && HopperCurrentTime < HopperStartTime + 2*HopperDelay){
    HopperServo.write(90);
  }
  if(HopperStartTime > HopperStartTime + 2*HopperDelay){
    HopperServo.write(180);
  }
}

//------------Stirring Motor Controls-----------
void MotorOn(void){
  analogWrite(MOTOR, 0); //Relay Pin
}

void MotorOff(void){
  analogWrite(MOTOR, 255); //Relay Pin
}

//------------Peltier Chip Controls-----------
void PeltierOn(void){
  analogWrite(RELAY1, 0);
  analogWrite(RELAY2, 0);
  analogWrite(RELAY3, 0);
  analogWrite(RELAY4, 0);
}

void PeltierOff(void){
  analogWrite(RELAY1, 255);
  analogWrite(RELAY2, 255);
  analogWrite(RELAY3, 255);
  analogWrite(RELAY4, 255);
}
