#include <Wire.h>
#include <EVShield.h>
#include <EVs_NXTTouch.h>


// setup for this example:
// horizontal motor:  bank A, M1
// vertical motor:    bank A, M2
// gripper motor:     bank B, M1
// touch sensor:      bank B, BBS1

// Open the Serial terminal to view.

// declare the EVShield(s) attached to your Arduino.
EVShield    evshield(0x34, 0x36);

// declare the i2c devices used on EVShield(s).

// declare analog devices attached to evshields.



int press_counter = 0;
int32_t posA1, posA2, posB1, posB2;
int motor_speed = 70, gripper_degree = 130, offset_error_horizontal = 50;

EVs_NXTTouch    touch1;

void setup()
{
  char str[256];

  Serial.begin(115200);  // start serial for output
  delay(500); // wait, allowing time to activate the serial monitor

  Serial.println ("Initializing the devices ...");

  // EVShield provides multiple protocols
  // Initialize the protocol for EVShield
  // It is best to use Hardware I2C (unless you want to use Ultrasonic).

  //evshield.init( SH_SoftwareI2C );
  evshield.init(); // the default is SH_HardwareI2C

  // Wait until user presses GO button to continue the program
  Serial.println ("Turn the motor shaft to see changing encoder values.");
  Serial.println ("Press Left Arrow button to reset motor encoder.");
  Serial.println ("---------------------------");
  Serial.println ("Press GO button to continue");
  Serial.println ("---------------------------");
  while (!evshield.getButtonState(BTN_GO)) {
    if (millis() % 1000 < 3) {
      Serial.println("Press GO button to continue");
    }
  }

  // Initialize the i2c sensors.

  // initialize the analog sensors.
  touch1.init( &evshield, SH_BBS1 );

  // reset motors.
  evshield.bank_a.motorReset();
  evshield.bank_b.motorReset();
}

int32_t  new_encoder1, new_encoder2, new_encoder3;  //was outside the setup function in 'MotorEncoder' example

void loop()
{
  char aa[80];
  int a;
  char str[256];
  bool touch_status;
  bool last_status;

  delay (1000);
  new_encoder3 = evshield.bank_b.motorGetEncoderPosition(SH_Motor_1); //motor for gripper
  new_encoder2 = evshield.bank_a.motorGetEncoderPosition(SH_Motor_2); //motor for vertical
  new_encoder1 = evshield.bank_a.motorGetEncoderPosition(SH_Motor_1); //motor for horizontal

  sprintf (str, "encoder values: 1) %ld, 2) %ld, 3) %ld", new_encoder1, new_encoder2, new_encoder3);
  Serial.println (str);
  str[0] = '\0';

  a = touch1.readRaw(); //  reads if the status of the touch sensor

  touch_status = touch1.isPressed();  // places a boolean value for 'touch_status'
  //  'sprintf' is a more efficient way to print multiple things instead of using multiple 'Serial.print()'s

  sprintf (str, "touch1: is pressed : %s\nButton Counter: %d", touch_status ? "true" : "false", press_counter);
  Serial.println(str);
  str[0] = '\0';

  if ( touch_status != last_status ) {
    if (touch_status) {
      press_counter++;
      delay(1000);
      str[0] = '\0';

      switch (press_counter) {
        case 1: //save posA after first button press
          posA1 = new_encoder1;
          posA2 = new_encoder2;

          str[0] = '\0';
          sprintf (str, "case 1 encoder values: 1) %ld, 2) %ld", posA1, posA2);
          Serial.println(str);
          break;
        case 2: //save posB after second button press
          posB1 = new_encoder1;
          posB2 = new_encoder2;

          str[0] = '\0';
          sprintf (str, "case 2 encoder values: 1) %ld, 2) %ld", posB1, posB2);
          Serial.println(str);
          break;
        case 3: //run automatic movement after third button press
          press_counter = 0;
          sprintf (str, "case 3 posA: 1) %ld, 2) %ld,  posB: 1) %ld, 2) %ld, ", posA1, posA2, posB1, posB2);
          Serial.println(str);

          //move upwards
          evshield.bank_a.motorRunDegrees(SH_Motor_2, SH_Direction_Forward, motor_speed, 60, SH_Completion_Wait_For, SH_Next_Action_Brake); //vertical motor
          delay(1000);

          //move to posA
          evshield.bank_a.motorRunDegrees(SH_Motor_1, SH_Direction_Forward, motor_speed, posA1 - posB1 + offset_error_horizontal, SH_Completion_Wait_For, SH_Next_Action_Brake); //horizontal motor
          delay(1000);
          evshield.bank_a.motorRunDegrees(SH_Motor_2, SH_Direction_Forward, motor_speed, posA2 - posB2, SH_Completion_Wait_For, SH_Next_Action_Brake); //vertical motor
          delay(1000);

          //move downwards
          evshield.bank_a.motorRunDegrees(SH_Motor_2, SH_Direction_Reverse, motor_speed-30, 40, SH_Completion_Wait_For, SH_Next_Action_Brake); //vertical motor
          delay(1000);

          //closes gripper
          evshield.bank_b.motorRunDegrees(SH_Motor_1, SH_Direction_Forward, motor_speed, gripper_degree, SH_Completion_Wait_For, SH_Next_Action_Brake); //gripper motor closes when fully open
          delay(1000);

          //move upwards
          evshield.bank_a.motorRunDegrees(SH_Motor_2, SH_Direction_Forward, motor_speed, 60, SH_Completion_Wait_For, SH_Next_Action_Brake); //vertical motor
          delay(1000);

          //move to posB
          evshield.bank_a.motorRunDegrees(SH_Motor_1, SH_Direction_Reverse, motor_speed, posA1 - posB1 + offset_error_horizontal, SH_Completion_Wait_For, SH_Next_Action_Brake); //horizontal motor
          delay(1000);
          evshield.bank_a.motorRunDegrees(SH_Motor_2, SH_Direction_Reverse, motor_speed, posA2 - posB2, SH_Completion_Wait_For, SH_Next_Action_Brake); //vertical motor
          delay(1000);

          //move downwards
          evshield.bank_a.motorRunDegrees(SH_Motor_2, SH_Direction_Reverse, motor_speed-30, 40, SH_Completion_Wait_For, SH_Next_Action_Brake); //vertical motor
          delay(1000);

          //opens gripper
          evshield.bank_b.motorRunDegrees(SH_Motor_1, SH_Direction_Reverse, motor_speed, gripper_degree, SH_Completion_Wait_For, SH_Next_Action_Brake); //gripper motor opens
          delay(1000);

          //move upwards
          evshield.bank_a.motorRunDegrees(SH_Motor_2, SH_Direction_Forward, motor_speed, posA2 - posB2 + 50, SH_Completion_Wait_For, SH_Next_Action_Brake); //vertical motor
          delay(1000);

          break;
        default:
          sprintf (str, "Error. Encoder values: 1) %ld, 2) %ld, 3) %ld", new_encoder1, new_encoder2, new_encoder3);
          Serial.println(str);
          break;
      }
    }
    last_status = touch_status;
  }
}
