#include <stdio.h>
#include "../../car_api.h"


int main(int argc, char *argv[]){
  int pins[4]={PINS_DEFAULT};

  printf("[%d %d %d %d]\n",pins[0],pins[1],pins[2],pins[3]);
  struct Car new_car= setup_new_car(pins[0],pins[1],pins[2],pins[3]); //WiringPi NUMBERS, NOT BCM/PIN NUMBER

  if(argc==0){// default action on car
	printf("Car started moving using default actions\r\n");
	move_car_fwd(new_car);
	sleep(4);
	move_car_left(new_car);
	sleep(4);
	move_car_right(new_car);
	sleep(4);
	move_car_bwd(new_car);
	sleep(4);
	stop_car(new_car);
  } else if(argv[0]="use_dir"){


	for(;;){
	  printf( "\r\nEnter a direction : (press i for instructions)");
	  int dir= getchar();
	  switch(dir){
	  case 'f':
		move_car_fwd(new_car);
		printf("moving forward");
		break;
	  case 'b':
		move_car_bwd(new_car);
		printf("moving backward");
		break;
	  case 'l':
		move_car_left(new_car);
		printf("moving left");
		break;
	  case 'r':
		move_car_right(new_car);
		printf("moving right");
		break;
	  case 's':
		stop_car(new_car);
		printf("stopped car");
		break;
	  case 'e':
		stop_car(new_car);
		printf("Car program ended\r\n");
		return 0;
		break;
	  case 'i':
		printf("Use f,b,l,r,s to go forward, backwards, left, right, stop respectively. e to end program\r\n");
		break;
	  }

	}
  }




  printf("Car program ended\r\n");
  return 0;
}
