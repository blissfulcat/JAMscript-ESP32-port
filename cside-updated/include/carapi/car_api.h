#if  defined __has_include
#if __has_include ("pins.h")
#include "pins.h"
#else
#define PINS_DEFAULT 0,2,3,4
#endif
#else
#define PINS_DEFAULT 0,2,3,4
#endif

#if defined __has_include
#if __has_include (<wiringPi.h>)
#include <wiringPi.h>
#else
#define MOCK_WIRINGPI
#endif
#else
#define MOCK_WIRINGPI
#endif

struct Car{
  int left_pin_0;
  int left_pin_1;
  int right_pin_0;
  int right_pin_1;
};

struct Car setup_new_car(int lp0,int lp1,int rp0,int rp1){//specify which pins correspond to which side of the car
  struct Car new_car;
  new_car.left_pin_0=lp0;
  new_car.left_pin_1=lp1;
  new_car.right_pin_0=rp0;
  new_car.right_pin_1=rp1;
#ifndef MOCK_WIRINGPI
  wiringPiSetup();// sets up the wiring pi library
  pinMode(lp0,OUTPUT);//setup each pin to output mode
  pinMode(lp1,OUTPUT);
  pinMode(rp0,OUTPUT);
  pinMode(rp1,OUTPUT);
#else
  printf("Mocking WiringPI\n");
#endif
  return new_car;
}

struct Car setup_default_car(){
  return setup_new_car(PINS_DEFAULT);
}

void move_car_fwd(struct Car car){//moves the car forward at max motor speed
#ifndef MOCK_WIRINGPI
  digitalWrite(car.left_pin_0,HIGH);
  digitalWrite(car.left_pin_1,LOW);
  digitalWrite(car.right_pin_0,HIGH);
  digitalWrite(car.right_pin_1,LOW);
#endif
}
void move_car_bwd(struct Car car){//moves the car backward
#ifndef MOCK_WIRINGPI
  digitalWrite(car.left_pin_0,LOW);
  digitalWrite(car.left_pin_1,HIGH);
  digitalWrite(car.right_pin_0,LOW);
  digitalWrite(car.right_pin_1,HIGH);
#endif
}

void stop_car(struct Car car){//stop the vehicle
#ifndef MOCK_WIRINGPI
  digitalWrite(car.left_pin_0,LOW);
  digitalWrite(car.left_pin_1,LOW);
  digitalWrite(car.right_pin_0,LOW);
  digitalWrite(car.right_pin_1,LOW);
#endif
}

void move_car_left(struct Car car){//moves the car left
#ifndef MOCK_WIRINGPI
  digitalWrite(car.left_pin_0,LOW);
  digitalWrite(car.left_pin_1,LOW);
  digitalWrite(car.right_pin_0,HIGH);
  digitalWrite(car.right_pin_1,LOW);
#endif
}

void move_car_right(struct Car car){// moves the car right
#ifndef MOCK_WIRINGPI
  digitalWrite(car.left_pin_0,HIGH);
  digitalWrite(car.left_pin_1,LOW);
  digitalWrite(car.right_pin_0,LOW);
  digitalWrite(car.right_pin_1,LOW);
#endif
}
