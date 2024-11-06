#ifndef __CARCTL_H__
#define __CARCTL_H__

#define CAR_SUCCESS 1
#define CAR_FAILURE 0

#define CAR_I2C_ADDR 0x40
#define CAR_RESET_REG 0x00
#define CAR_PWM0_ON_L 0x06
#define CAR_PWM0_ON_H 0x07
#define CAR_PWM0_OFF_L 0x08
#define CAR_PWM0_OFF_H 0x09
#define CAR_PWM_SIZE 4

#define CAR_SERVO_ADDR 0
#define CAR_MOTOR_ADDR 1

#define PWM_MAX 4095

// Car Contexty Constructor Description
typedef struct _car_context_descriptor_t{
  // These are using non-normalized steering values
  int steering_center; //Normally around 1100
  int steering_range;


  int throttle_offset;
  int throttle_range;
  int throttle_reverse_delta;
  float throttle_reverse_scale;
  
} car_context_descriptor_t;

typedef struct _car_context_t{
  int steering_center;
  int steering_range;

  // normalized steering value 0-1, 0.5 is centered
  float current_steering;


  // Needs to be changed.
  int throttle_offset;
  int throttle_range;
  int throttle_reverse_delta;
  float throttle_reverse_scale;
  

  float current_throttle;

  int i2c_fd;
  
} car_context_t;

int car_create(car_context_t* ctx, car_context_descriptor_t descriptor);

// Steering values normalized from -1-1
int car_steer(car_context_t* ctx, float val);
int car_throttle(car_context_t* ctx, float val);


#ifdef CARCTL_IMPLEMENTATION
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>

// Linux File Headers
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>

// I2C Headers
#include <linux/i2c-dev.h>



#define PROP(r, x) {r = x; if(r!=CAR_SUCCESS){return r;}}


int _car_write_pwm_i2c(car_context_t* ctx, uint8_t data_addr, uint8_t value) {
  uint8_t buf[2];

  buf[0] = data_addr;
  buf[1] = value;
    
  if(write(ctx->i2c_fd, buf, 2) != 2) {
    printf("failed to write stuff\n");
    return CAR_FAILURE;
  }

  return CAR_SUCCESS;
}

int _car_set_pwm(car_context_t* ctx, int channel, int on, int off) {
  int res;
  PROP(res, _car_write_pwm_i2c(ctx, CAR_PWM0_ON_L+CAR_PWM_SIZE*channel,  on & 0xFF));
  PROP(res, _car_write_pwm_i2c(ctx, CAR_PWM0_ON_H+CAR_PWM_SIZE*channel,  on >> 8));
  PROP(res, _car_write_pwm_i2c(ctx, CAR_PWM0_OFF_L+CAR_PWM_SIZE*channel, off & 0xFF));
  PROP(res, _car_write_pwm_i2c(ctx, CAR_PWM0_OFF_H+CAR_PWM_SIZE*channel, off >> 8));
  return CAR_SUCCESS;
}

int car_create(car_context_t* ctx, car_context_descriptor_t descriptor) {
  assert(ctx!=NULL);
  ctx->current_steering = 0.5;
  ctx->steering_center = descriptor.steering_center;
  ctx->steering_range  = descriptor.steering_range;
  ctx->throttle_offset = descriptor.throttle_offset;
  ctx->throttle_range  = descriptor.throttle_range;
  ctx->throttle_reverse_delta = descriptor.throttle_reverse_delta;
  ctx->throttle_reverse_scale = descriptor.throttle_reverse_scale;
  
  ctx->i2c_fd = open("/dev/i2c-1", O_RDWR);
  if(!ctx->i2c_fd) {
    printf("Failed to find I2C adapter interface.\n");
    return CAR_FAILURE;
  }

  if(ioctl(ctx->i2c_fd, I2C_SLAVE, CAR_I2C_ADDR) < 0) {
    printf("Failed to select our I2C device.\n");
    return CAR_FAILURE;
  }

  if(_car_write_pwm_i2c(ctx, CAR_RESET_REG, 0x00) != CAR_SUCCESS) {
    printf("Failed to reset I2C Device");
    return CAR_FAILURE;
  }

  // Callibrate Throttle.
  _car_set_pwm(ctx, CAR_MOTOR_ADDR, 0, ctx->throttle_offset);
  ctx->current_throttle = 0.0;

  return CAR_SUCCESS;
}

float _clamp_norm(float val) {
  if(val>1)
    return 1;
  if(val<-1)
    return -1;
  return val;
}

int car_steer(car_context_t* ctx, float val) {
  int res;

  int real_val = _clamp_norm(val)*ctx->steering_range + ctx->steering_center;
  
  PROP(res, _car_set_pwm(ctx, CAR_SERVO_ADDR, 0, real_val));
  
  ctx->current_steering = _clamp_norm(val);

  return CAR_SUCCESS;
}

int car_throttle(car_context_t* ctx, float val) {
  int res;

  // throttle range for reverse is half that of forwards
  float rscale = (val < 0) ? ctx->throttle_reverse_scale : 1.0;
  float roff   = (val < 0) ? ctx->throttle_reverse_delta: 0.0;
  
  int real_val = ctx->throttle_range * _clamp_norm(val) / rscale - roff + ctx->throttle_offset; 
  
  printf("Setting Throttle To: %d     %f\n", real_val, val);

  PROP(res, _car_set_pwm(ctx, CAR_MOTOR_ADDR, 0, real_val));

  ctx->current_throttle = _clamp_norm(val);

  return CAR_SUCCESS;
}

// Numbers:
// Center: 1130
// 930 THING
// RANGE: 200
// 


#endif

#endif
