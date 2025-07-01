#ifndef MPU6050_H
#define MPU6050_H

#include "main.h"

#define MPU6050_ADDR 0xD0  /* 7-bit address 0x68, left-shifted by 1 for HAL */

#define MPU6050_SMPLRT_DIV 0x19
#define MPU6050_CONFIG 0x1A
#define MPU6050_GYRO_CONFIG 0x1B
#define MPU6050_ACCEL_CONFIG 0x1C
#define MPU6050_PWR_MGMT_1 0x6B
#define MPU6050_ACCEL_XOUT_H 0x3B

typedef struct {
  float accel_x;
  float accel_y;
  float accel_z;
  float temp;
  float gyro_x;
  float gyro_y;
  float gyro_z;
} MPU6050_DataTypedef;

MPU6050_DataTypedef* MPU6050_GetData(void);

int MPU6050_Init(void);
int MPU6050_Read_Data(void);

#endif
