#include <stdio.h>
#include "../../car_api.h"

#define PINS {0,2,3,4}

int main(int argc,char*argv[]){
  struct Car new_car;
  for(int i=0;i<24;i++){
	int pins[4]=PINS,ii=i;
	for(int j=0;j<3;j++){
	  int tmp=pins[j],tsw=j+ii%(4-j);
	  pins[j]=pins[tsw];
	  pins[tsw]=tmp;
	  ii/=4-j;
	}
	printf("[%d %d %d %d]\n",pins[0],pins[1],pins[2],pins[3]);
	new_car=setup_new_car(pins[0],pins[1],pins[2],pins[3]); //WiringPi NUMBERS, NOT BCM/PIN NUMBER
	for(;;){
	  printf( "\nEnter a direction : (press i for instructions)");
	  int dir;
	  do{
		dir=getchar();
	  }while(dir=='\n');
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
	  case 'n':
		stop_car(new_car);
		goto nextpins;
	  case 'e':
		stop_car(new_car);
		printf("Car program ended\n");
		return 0;
	  case 'i':
		printf("Use f,b,l,r,s to go forward, backwards, left, right, stop respectively. n for next pin permutation, e to end program");
	  }
	}
	nextpins:printf("next config... ");
  }
  stop_car(new_car);
  return 0;
}
