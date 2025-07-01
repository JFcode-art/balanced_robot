#ifndef FREERTOS_APP_H
#define FREERTOS_APP_H

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

#define PRIORITY_SENSOR        (configMAX_PRIORITIES - 1)
#define PRIORITY_ATTITUDE      (configMAX_PRIORITIES - 2)
#define PRIORITY_CONTROL       (configMAX_PRIORITIES - 3)
#define PRIORITY_MOTOR_STATUS  (configMAX_PRIORITIES - 4)
#define PRIORITY_BLUETOOTH     (configMAX_PRIORITIES - 5)
#define PRIORITY_COMM          (configMAX_PRIORITIES - 6)

#define STACK_SIZE_SENSOR        128
#define STACK_SIZE_ATTITUDE      128
#define STACK_SIZE_CONTROL       512
#define STACK_SIZE_MOTOR_STATUS  128
#define STACK_SIZE_BLUETOOTH     256
#define STACK_SIZE_COMM          128

#define QUEUE_SENSOR_SIZE        10
#define QUEUE_ATTITUDE_SIZE      10
#define QUEUE_CONTROL_SIZE       10
#define QUEUE_MOTOR_STATUS_SIZE  5

#define BT_CMD_SET_SPEED         0x01
#define BT_CMD_SET_ANGLE         0x02
#define BT_CMD_SET_PID           0x03
#define BT_CMD_STOP              0x04
#define BT_CMD_GET_STATUS        0x05
#define BT_CMD_SET_POSITION      0x06

#define MOTOR_STATUS_NORMAL      0x00
#define MOTOR_STATUS_LEFT_OVER   0x01
#define MOTOR_STATUS_RIGHT_OVER  0x02

typedef struct {
  float accel_x;
  float accel_y;
  float accel_z;
  float gyro_x;
  float gyro_y;
  float gyro_z;
  TickType_t timestamp;
} SensorData_t;

typedef struct {
  float pitch;
  float roll;
  float yaw;
  TickType_t timestamp;
} AttitudeData_t;

typedef struct {
  float pitch;
  float target_speed;
  float target_position;
  float pwm_left;
  float pwm_right;
  float position;
  TickType_t timestamp;
} ControlData_t;

typedef struct {
  int32_t encoder_left;
  int32_t encoder_right;
  float speed_left;
  float speed_right;
  int16_t current_left;
  int16_t current_right;
  uint8_t status;
  TickType_t timestamp;
} MotorStatusData_t;

typedef struct {
  uint8_t cmd;
  uint8_t len;
  uint8_t data[32];
  TickType_t timestamp;
} BluetoothData_t;

typedef struct {
  float target_speed;
  float target_angle;
  float target_position;
  uint8_t control_mode;
} RemoteControlData_t;

extern TaskHandle_t TaskHandle_Sensor;
extern TaskHandle_t TaskHandle_Attitude;
extern TaskHandle_t TaskHandle_Control;
extern TaskHandle_t TaskHandle_MotorStatus;
extern TaskHandle_t TaskHandle_Bluetooth;
extern TaskHandle_t TaskHandle_Comm;

extern QueueHandle_t Queue_Sensor;
extern QueueHandle_t Queue_Attitude;
extern QueueHandle_t Queue_Control;
extern QueueHandle_t Queue_MotorStatus;

extern RemoteControlData_t RemoteControl;
extern SemaphoreHandle_t RemoteControl_Mutex;
extern StaticSemaphore_t RemoteControl_MutexBuffer;

extern StaticTask_t TaskBuffer_Sensor;
extern StackType_t Stack_Sensor[STACK_SIZE_SENSOR];

extern StaticTask_t TaskBuffer_Attitude;
extern StackType_t Stack_Attitude[STACK_SIZE_ATTITUDE];

extern StaticTask_t TaskBuffer_Control;
extern StackType_t Stack_Control[STACK_SIZE_CONTROL];

extern StaticTask_t TaskBuffer_MotorStatus;
extern StackType_t Stack_MotorStatus[STACK_SIZE_MOTOR_STATUS];

extern StaticTask_t TaskBuffer_Bluetooth;
extern StackType_t Stack_Bluetooth[STACK_SIZE_BLUETOOTH];

extern StaticTask_t TaskBuffer_Comm;
extern StackType_t Stack_Comm[STACK_SIZE_COMM];

void FreeRTOS_Init(void);

#endif
