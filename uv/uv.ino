#include <Wire.h>
#include "LCD.h" // Para o LCD
#include "LiquidCrystal_I2C.h" // Para o LCD
#include <SparkFun_VEML6075_Arduino_Library.h>

LiquidCrystal_I2C lcd(0x27,2,1,0,4,5,6,7); //  Para o LCD

/* Constante para o tamanho do Display */
const int LCD_NB_ROWS = 4;
const int LCD_NB_COLUMNS = 20;

VEML6075 uv; // Create a VEML6075 object

// Calibration constants:
// Four gain calibration constants -- alpha, beta, gamma, delta -- can be used to correct the output in
// reference to a GOLDEN sample. The golden sample should be calibrated under a solar simulator.
// Setting these to 1.0 essentialy eliminates the "golden"-sample calibration
const float CALIBRATION_ALPHA_VIS = 1.0; // UVA / UVAgolden
const float CALIBRATION_BETA_VIS  = 1.0; // UVB / UVBgolden
const float CALIBRATION_GAMMA_IR  = 1.0; // UVcomp1 / UVcomp1golden
const float CALIBRATION_DELTA_IR  = 1.0; // UVcomp2 / UVcomp2golden

// Responsivity:
// Responsivity converts a raw 16-bit UVA/UVB reading to a relative irradiance (W/m^2).
// These values will need to be adjusted as either integration time or dynamic settings are modififed.
// These values are recommended by the "Designing the VEML6075 into an application" app note for 100ms IT
const float UVA_RESPONSIVITY = 0.00110; // UVAresponsivity
const float UVB_RESPONSIVITY = 0.00125; // UVBresponsivity

// UV coefficients:
// These coefficients
// These values are recommended by the "Designing the VEML6075 into an application" app note
const float UVA_VIS_COEF_A = 2.22; // a
const float UVA_IR_COEF_B  = 1.33; // b
const float UVB_VIS_COEF_C = 2.95; // c
const float UVB_IR_COEF_D  = 1.75; // d

void setup()
{
  Serial.begin(9600);

  if (uv.begin() == false) {
     Serial.println("Unable to communicate with VEML6075.");
  while (1) ;
    // Serial.println("Sensor OK");
  
  }
  
  // Integration time and high-dynamic values will change the UVA/UVB sensitivity. That means
  // new responsivity values will need to be measured for every combination of these settings.

  uv.setIntegrationTime(VEML6075::IT_100MS);
  uv.setHighDynamic(VEML6075::DYNAMIC_NORMAL);

  lcd.begin(LCD_NB_COLUMNS, LCD_NB_ROWS);
  lcd.setBacklightPin(3,POSITIVE); // BL, BL_POL
  lcd.setBacklight(HIGH);
  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print("Atencao !!!");
  lcd.setCursor(0, 1);
  lcd.print("Projeto Experimental");
  lcd.setCursor(0, 2);
  lcd.print("Medidor Indice U.V.");

   
   delay(5500);
   lcd.clear();
}

void loop()
{
  uint16_t rawA, rawB, visibleComp, irComp;
  float uviaCalc, uvibCalc, uvia, uvib, uvi;

  // Read raw and compensation data from the sensor
  rawA = uv.rawUva();
  rawB = uv.rawUvb();
  visibleComp = uv.visibleCompensation();
  irComp = uv.irCompensation();

  // Calculate the simple UVIA and UVIB. These are used to calculate the UVI signal.
  uviaCalc = (float)rawA - ((UVA_VIS_COEF_A * CALIBRATION_ALPHA_VIS * visibleComp) / CALIBRATION_GAMMA_IR)
                  - ((UVA_IR_COEF_B  * CALIBRATION_ALPHA_VIS * irComp) /  CALIBRATION_DELTA_IR);
  uvibCalc = (float)rawB - ((UVB_VIS_COEF_C * CALIBRATION_BETA_VIS * visibleComp) / CALIBRATION_GAMMA_IR)
                  - ((UVB_IR_COEF_D  * CALIBRATION_BETA_VIS * irComp) /  CALIBRATION_DELTA_IR);

  // Convert raw UVIA and UVIB to values scaled by the sensor responsivity
  uvia = uviaCalc * (1.0 / CALIBRATION_ALPHA_VIS) * UVA_RESPONSIVITY;
  uvib = uvibCalc * (1.0 / CALIBRATION_BETA_VIS) * UVB_RESPONSIVITY;

  // Use UVIA and UVIB to calculate the average UVI:
  uvi = (uvia + uvib) / 2.0;

 lcd.setCursor(0, 0);
 lcd.print("uvI  " + String(uvi));
 Serial.println(String(uvi));

  //Serial.println(String(uvi));

  lcd.setCursor(0, 2);
  lcd.print("                    "); 

  lcd.setCursor(0, 1);
  lcd.print("                    "); 
    
  if(uvi >0.01 and uvi <= 0.7){
    lcd.setCursor(0, 1);
    lcd.print("Zone 1 Shade Method");
  }
  else if(uvi > 0.7 and uvi <=1){
    lcd.setCursor(0, 1);
    lcd.print("Zone 2 Shade Method");
  }
  else if(uvi > 1 and uvi <=3){
    lcd.setCursor(0, 1);
    lcd.print("Zone 2 Sunbeam Meth.");
  }
  else if(uvi >3 and uvi <=4.5){  
    lcd.setCursor(0, 1);
    lcd.print("Zone 3 Sunbeam Meth.");
  }
  else if(uvi >4.5 and uvi <= 9.5){
    lcd.setCursor(0, 1);
    lcd.print("Zone 3 Sunbeam Meth.");
    lcd.setCursor(0, 2);
    lcd.print("Zone 4 Sunbeam Meth.");
  }
  else if(uvi > 9.5){
    lcd.setCursor(0, 1);
    lcd.print("UVI EXTREME !!!");
    lcd.setCursor(0, 2);
    lcd.print("DANGER !!!");
  }  

  
  lcd.setCursor(0, 3);
  lcd.print("uvaI ");
  lcd.setCursor(5, 3);
  lcd.print(uvia);
  lcd.setCursor(10, 3);
  lcd.print("uvbI ");
  lcd.setCursor(15, 3);
  lcd.print(uvib);
   
  delay(1500);
}
