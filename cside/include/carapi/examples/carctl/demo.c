#define CARCTL_IMPLEMENTATION
#include "carapi/carctl.h"

#include <stdio.h>
#include <assert.h>

#include <unistd.h>
#include <termios.h>

char getch() {
  char buf = 0;
  struct termios old = {0};
  if(tcgetattr(0, &old) < 0)
    return 0; //OOP

  old.c_lflag &= ~ICANON;
  old.c_lflag &= ~ECHO;
  old.c_cc[VMIN] = 1;
  old.c_cc[VTIME] = 0;
  if (tcsetattr(0, TCSANOW, &old) < 0)
    return 0; // OOP
  if (read(0, &buf, 1) < 0)
    return 0;
  old.c_lflag |= ICANON;
  old.c_lflag |= ECHO;
  if (tcsetattr(0, TCSADRAIN, &old) < 0)
    return 0; // OOP
  
  return buf;
}

int main() {

  car_context_descriptor_t descriptor;
  descriptor.steering_center = 1130;
  descriptor.steering_range = 200;
  descriptor.throttle_offset = 1000;//-0.25;
  descriptor.throttle_range = 700;//0.50;
  descriptor.throttle_reverse_delta = 30;
  descriptor.throttle_reverse_scale = 8;

  car_context_t ctx;
  assert(car_create(&ctx, descriptor)==CAR_SUCCESS);

  printf("Use Arrow Keys to Drive, press 'q' to exit.\n");
  
  while(1) {
    switch(getch()) {
    case '\033':
      getch();
      switch(getch()) {
      case 'A': // Up Arrow
	car_throttle(&ctx, ctx.current_throttle+0.05);
	break;
      case 'B': // Down Arrow
	car_throttle(&ctx, ctx.current_throttle-0.05);
	break;
      case 'C': // Right Arrow
	car_steer(&ctx, ctx.current_steering+0.10);
	break;
      case 'D': // Left Arrow
	car_steer(&ctx, ctx.current_steering-0.10);
	break;
      }
      break;
    case 'q':
      printf("Exiting Demo...\n");
      return 0;
      break;
    }
  }

}


