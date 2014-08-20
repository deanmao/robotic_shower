#include <AutoDriver.h>
#include <Servo.h>

// Pins:
#define CW_LIMIT 4
#define CCW_LIMIT 5
#define KILL_SWITCH 3
#define GO_BUTTON 2
#define SERVO_PIN 7

// Constants:
#define CW 1
#define CCW 0

// Variables:
AutoDriver motor(10, 6);
Servo triggerServo;
int direction = CW;
int running = 1;
int spraying = 0;
int cycles = 0;

void setup()
{
  // These config variables are from the example library code:
  //
  motor.configSyncPin(BUSY_PIN, 0);// BUSY pin low during operations;
                                    //  second paramter ignored.
  motor.configStepMode(STEP_FS);   // 0 microsteps per step
  motor.setMaxSpeed(10000);        // 10000 steps/s max
  motor.setFullSpeed(10000);       // microstep below 10000 steps/s
  motor.setAcc(10000);             // accelerate at 10000 steps/s/s
  motor.setDec(10000);
  motor.setSlewRate(SR_530V_us);   // Upping the edge speed increases torque.
  motor.setOCThreshold(OC_1125mA);  // OC threshold 750mA
  motor.setPWMFreq(PWM_DIV_2, PWM_MUL_2); // 31.25kHz PWM freq
  motor.setOCShutdown(OC_SD_DISABLE); // don't shutdown on OC
  motor.setVoltageComp(VS_COMP_DISABLE); // don't compensate for motor V
  motor.setSwitchMode(SW_USER);    // Switch is not hard stop
  motor.setOscMode(INT_16MHZ_OSCOUT_16MHZ); // for motor, we want 16MHz
                                    //  internal osc, 16MHz out. boardB and
                                    //  boardC will be the same in all respects
                                    //  but this, as they will bring in and
                                    //  output the clock to keep them
                                    //  all in phase.
  motor.setAccKVAL(255);           // We'll tinker with these later, if needed.
  motor.setDecKVAL(255);
  motor.setRunKVAL(255);
  motor.setHoldKVAL(32);           // This controls the holding current; keep it low.

  // Disable the use of the reset pin because we don't care
  motor.resetDev();

  // Setup the input pins
  pinMode(CW_LIMIT, INPUT);
  pinMode(CCW_LIMIT, INPUT);
  pinMode(KILL_SWITCH, INPUT);
  pinMode(GO_BUTTON, INPUT);
  triggerServo.attach(SERVO_PIN);
}

void stop() {
  motor.softStop();
  while (motor.busyCheck());
  running = 0;
}

void spray() {
  if (!spraying) {
    for(int pos = 0; pos < 180; pos += 1) {
      triggerServo.write(pos);
      delay(15);
    }
    spraying = 1;
  }
}

void stopSpraying() {
  if (spraying) {
    for(int pos = 180; pos >= 1; pos -= 1) {
      triggerServo.write(pos);
      delay(15);
    }
    spraying = 0;
  }
}

void checkAndGo() {
  if (digitalRead(CCW_LIMIT) == HIGH) {
    direction = CW;
    if (running) stop();
    go();
    cycles--;
    delay(1000);  // hopefully enough time to move away from switch
  }
  if (digitalRead(CW_LIMIT) == HIGH) {
    direction = CCW;
    if (running) stop();
    go();
    cycles--;
    delay(1000);  // hopefully enough time to move away from switch
  }
  if (!spraying) spray();
  if (!running) go();
}

void go() {
  if (direction == CW) {
    motor.run(FWD, 1000);
  } else {
    motor.run(REV, 1000);
  }
  running = 1;
}

void loop() {
  if (digitalRead(GO_BUTTON) == HIGH) {
    cycles = 3;
  }
  if (digitalRead(KILL_SWITCH) == HIGH && cycles > 0) {
    checkAndGo();
  } else {
    if (running) stop();
    if (spraying) stopSpraying();
  }
}

