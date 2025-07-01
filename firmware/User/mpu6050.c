#include "mpu6050.h"
#include "FreeRTOS.h"
#include "task.h"

static MPU6050_DataTypedef MPU6050_Data;

MPU6050_DataTypedef* MPU6050_GetData(void) {
  return &MPU6050_Data;
}

static int MPU6050_WriteReg(uint8_t reg, uint8_t data) {
  return HAL_I2C_Mem_Write(&hi2c2, MPU6050_ADDR, reg, 1, &data, 1, 100) == HAL_OK ? 0 : -1;
}

static int MPU6050_ReadReg(uint8_t reg, uint8_t *data) {
  return HAL_I2C_Mem_Read(&hi2c2, MPU6050_ADDR, reg, 1, data, 1, 100) == HAL_OK ? 0 : -1;
}

int MPU6050_Init(void) {
  if (xTaskGetSchedulerState() == taskSCHEDULER_RUNNING) {
    vTaskDelay(pdMS_TO_TICKS(50));
  } else {
    HAL_Delay(50);
  }

  if (MPU6050_WriteReg(MPU6050_PWR_MGMT_1, 0x00) != 0) return -1;
  if (xTaskGetSchedulerState() == taskSCHEDULER_RUNNING) {
    vTaskDelay(pdMS_TO_TICKS(5));
  } else {
    HAL_Delay(5);
  }

  MPU6050_WriteReg(MPU6050_SMPLRT_DIV, 0x07);
  MPU6050_WriteReg(MPU6050_CONFIG, 0x00);
  MPU6050_WriteReg(MPU6050_GYRO_CONFIG, 0x18);
  MPU6050_WriteReg(MPU6050_ACCEL_CONFIG, 0x08);
  return 0;
}

int MPU6050_Read_Data(void) {
  uint8_t buffer[14];

  if (HAL_I2C_Mem_Read(&hi2c2, MPU6050_ADDR, MPU6050_ACCEL_XOUT_H, 1, buffer, 14, 100) != HAL_OK) {
    return -1;
  }

  int16_t raw_x = (int16_t)((uint16_t)(buffer[0] << 8) | buffer[1]);
  int16_t raw_y = (int16_t)((uint16_t)(buffer[2] << 8) | buffer[3]);
  int16_t raw_z = (int16_t)((uint16_t)(buffer[4] << 8) | buffer[5]);

  MPU6050_Data.accel_x = raw_x / 8192.0f;
  MPU6050_Data.accel_y = raw_y / 8192.0f;
  MPU6050_Data.accel_z = raw_z / 8192.0f;

  MPU6050_Data.temp = ((int16_t)((uint16_t)(buffer[6] << 8) | buffer[7])) / 340.0f + 36.53f;

  int16_t raw_gx = (int16_t)((uint16_t)(buffer[8] << 8) | buffer[9]);
  int16_t raw_gy = (int16_t)((uint16_t)(buffer[10] << 8) | buffer[11]);
  int16_t raw_gz = (int16_t)((uint16_t)(buffer[12] << 8) | buffer[13]);

  MPU6050_Data.gyro_x = raw_gx / 16.4f;
  MPU6050_Data.gyro_y = raw_gy / 16.4f;
  MPU6050_Data.gyro_z = raw_gz / 16.4f;
  return 0;
}
