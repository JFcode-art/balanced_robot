#include "freertos_app.h"
#include "motor.h"
#include "pid.h"
#include "filter.h"
#include "mpu6050.h"
#include "tim.h"
#include "usart.h"
#include "semphr.h"

extern IWDG_HandleTypeDef hiwdg;

TaskHandle_t TaskHandle_Sensor = NULL;
TaskHandle_t TaskHandle_Attitude = NULL;
TaskHandle_t TaskHandle_Control = NULL;
TaskHandle_t TaskHandle_MotorStatus = NULL;
TaskHandle_t TaskHandle_Bluetooth = NULL;
TaskHandle_t TaskHandle_Comm = NULL;

QueueHandle_t Queue_Sensor = NULL;
QueueHandle_t Queue_Attitude = NULL;
QueueHandle_t Queue_Control = NULL;
QueueHandle_t Queue_MotorStatus = NULL;

StaticQueue_t QueueBuffer_Sensor;
StaticQueue_t QueueBuffer_Attitude;
StaticQueue_t QueueBuffer_Control;
StaticQueue_t QueueBuffer_MotorStatus;

uint8_t QueueStorage_Sensor[QUEUE_SENSOR_SIZE * sizeof(SensorData_t)];
uint8_t QueueStorage_Attitude[QUEUE_ATTITUDE_SIZE * sizeof(AttitudeData_t)];
uint8_t QueueStorage_Control[QUEUE_CONTROL_SIZE * sizeof(ControlData_t)];
uint8_t QueueStorage_MotorStatus[QUEUE_MOTOR_STATUS_SIZE * sizeof(MotorStatusData_t)];

RemoteControlData_t RemoteControl = {0};
SemaphoreHandle_t RemoteControl_Mutex = NULL;
StaticSemaphore_t RemoteControl_MutexBuffer;

StaticTask_t TaskBuffer_Sensor;
StackType_t Stack_Sensor[STACK_SIZE_SENSOR];

StaticTask_t TaskBuffer_Attitude;
StackType_t Stack_Attitude[STACK_SIZE_ATTITUDE];

StaticTask_t TaskBuffer_Control;
StackType_t Stack_Control[STACK_SIZE_CONTROL];

StaticTask_t TaskBuffer_MotorStatus;
StackType_t Stack_MotorStatus[STACK_SIZE_MOTOR_STATUS];

StaticTask_t TaskBuffer_Bluetooth;
StackType_t Stack_Bluetooth[STACK_SIZE_BLUETOOTH];

StaticTask_t TaskBuffer_Comm;
StackType_t Stack_Comm[STACK_SIZE_COMM];

static void Task_Sensor(void *argument);
static void Task_Attitude(void *argument);
static void Task_Control(void *argument);
static void Task_MotorStatus(void *argument);
static void Task_Bluetooth(void *argument);
static void Task_Comm(void *argument);

void FreeRTOS_Init(void) {
  RemoteControl.target_speed = 0.0f;
  RemoteControl.target_angle = 0.0f;
  RemoteControl.target_position = 0.0f;

  RemoteControl_Mutex = xSemaphoreCreateMutexStatic(&RemoteControl_MutexBuffer);

  Queue_Sensor = xQueueCreateStatic(QUEUE_SENSOR_SIZE, sizeof(SensorData_t),
                                    QueueStorage_Sensor, &QueueBuffer_Sensor);
  Queue_Attitude = xQueueCreateStatic(QUEUE_ATTITUDE_SIZE, sizeof(AttitudeData_t),
                                      QueueStorage_Attitude, &QueueBuffer_Attitude);
  Queue_Control = xQueueCreateStatic(QUEUE_CONTROL_SIZE, sizeof(ControlData_t),
                                     QueueStorage_Control, &QueueBuffer_Control);
  Queue_MotorStatus = xQueueCreateStatic(QUEUE_MOTOR_STATUS_SIZE, sizeof(MotorStatusData_t),
                                         QueueStorage_MotorStatus, &QueueBuffer_MotorStatus);

  xTaskCreateStatic(Task_Sensor, "SensorTask", STACK_SIZE_SENSOR,
                    NULL, PRIORITY_SENSOR, Stack_Sensor, &TaskBuffer_Sensor,
                    &TaskHandle_Sensor);
  
  xTaskCreateStatic(Task_Attitude, "AttitudeTask", STACK_SIZE_ATTITUDE,
                    NULL, PRIORITY_ATTITUDE, Stack_Attitude, &TaskBuffer_Attitude,
                    &TaskHandle_Attitude);
  
  xTaskCreateStatic(Task_Control, "ControlTask", STACK_SIZE_CONTROL,
                    NULL, PRIORITY_CONTROL, Stack_Control, &TaskBuffer_Control,
                    &TaskHandle_Control);
  
  xTaskCreateStatic(Task_MotorStatus, "MotorStatusTask", STACK_SIZE_MOTOR_STATUS,
                    NULL, PRIORITY_MOTOR_STATUS, Stack_MotorStatus, &TaskBuffer_MotorStatus,
                    &TaskHandle_MotorStatus);
  
  xTaskCreateStatic(Task_Bluetooth, "BluetoothTask", STACK_SIZE_BLUETOOTH,
                    NULL, PRIORITY_BLUETOOTH, Stack_Bluetooth, &TaskBuffer_Bluetooth,
                    &TaskHandle_Bluetooth);
  
  xTaskCreateStatic(Task_Comm, "CommTask", STACK_SIZE_COMM,
                    NULL, PRIORITY_COMM, Stack_Comm, &TaskBuffer_Comm,
                    &TaskHandle_Comm);
  
  vTaskStartScheduler();
}

static void Task_Sensor(void *argument) {
  SensorData_t sensor_data;
  MPU6050_DataTypedef *mpu;

  if (MPU6050_Init() != 0) {
    Error_Handler();
  }

  while(1) {
    if (MPU6050_Read_Data() != 0) {
      vTaskDelay(pdMS_TO_TICKS(10));
      continue;
    }
    mpu = MPU6050_GetData();
    
    sensor_data.accel_x = mpu->accel_x;
    sensor_data.accel_y = mpu->accel_y;
    sensor_data.accel_z = mpu->accel_z;
    sensor_data.gyro_x = mpu->gyro_x;
    sensor_data.gyro_y = mpu->gyro_y;
    sensor_data.gyro_z = mpu->gyro_z;
    sensor_data.timestamp = xTaskGetTickCount();
    
    xQueueSend(Queue_Sensor, &sensor_data, portMAX_DELAY);
    
    vTaskDelay(pdMS_TO_TICKS(1));
  }
}

static void Task_Attitude(void *argument) {
  SensorData_t sensor_data;
  AttitudeData_t attitude_data;
  float angle = 0.0f;
  
  while(1) {
    if(xQueueReceive(Queue_Sensor, &sensor_data, portMAX_DELAY) == pdTRUE) {
      angle = Complementary_Filter(sensor_data.accel_y, sensor_data.accel_z, sensor_data.gyro_x);
      
      attitude_data.pitch = angle;
      attitude_data.roll = 0.0f;
      attitude_data.yaw = 0.0f;
      attitude_data.timestamp = sensor_data.timestamp;
      
      xQueueSend(Queue_Attitude, &attitude_data, portMAX_DELAY);
    }
    
    vTaskDelay(pdMS_TO_TICKS(1));
  }
}

static void Task_Control(void *argument) {
  AttitudeData_t attitude_data;
  ControlData_t control_data;
  
  PID_HandleTypeDef pid_angle;
  PID_HandleTypeDef pid_speed;
  PID_HandleTypeDef pid_position;
  
  float motor_speed = 0.0f;
  float position = 0.0f;
  int32_t encoder_count_left = 0;
  int32_t encoder_count_right = 0;
  
  PID_Init(&pid_angle, 20.0f, 0.0f, 2.0f, 100.0f, -100.0f);
  PID_Init(&pid_speed, 3.0f, 0.2f, 0.0f, 50.0f, -50.0f);
  PID_Init(&pid_position, 1.0f, 0.05f, 0.0f, 10.0f, -10.0f);
  
  Motor_Init();
  Encoder_Init();
  
  while(1) {
    if(xQueueReceive(Queue_Attitude, &attitude_data, portMAX_DELAY) == pdTRUE) {
      encoder_count_left = __HAL_TIM_GET_COUNTER(&htim2);
      encoder_count_right = __HAL_TIM_GET_COUNTER(&htim4);
      
      float delta_encoder = (encoder_count_left + encoder_count_right) / 2.0f;
      motor_speed = delta_encoder * 0.1f;
      position += delta_encoder * 0.01f;

      if (position > 1000.0f) position = 1000.0f;
      if (position < -1000.0f) position = -1000.0f;
      
      xSemaphoreTake(RemoteControl_Mutex, portMAX_DELAY);
      float position_error = RemoteControl.target_position - position;
      float speed_target = PID_Compute(&pid_position, position_error);
      speed_target += RemoteControl.target_speed;

      float angle_error = RemoteControl.target_angle - attitude_data.pitch;
      xSemaphoreGive(RemoteControl_Mutex);
      float angle_output = PID_Compute(&pid_angle, angle_error);
      
      float speed_error = speed_target - motor_speed;
      float speed_output = PID_Compute(&pid_speed, speed_error);
      
      float pwm = angle_output + speed_output;
      
      if(pwm > 1000) pwm = 1000;
      if(pwm < -1000) pwm = -1000;
      
      Motor_SetSpeed(MOTOR_LEFT, (int16_t)pwm);
      Motor_SetSpeed(MOTOR_RIGHT, (int16_t)pwm);
      
      __HAL_TIM_SET_COUNTER(&htim2, 0);
      __HAL_TIM_SET_COUNTER(&htim4, 0);
      
      control_data.pitch = attitude_data.pitch;
      control_data.target_speed = speed_target;
      xSemaphoreTake(RemoteControl_Mutex, portMAX_DELAY);
      control_data.target_position = RemoteControl.target_position;
      xSemaphoreGive(RemoteControl_Mutex);
      control_data.pwm_left = pwm;
      control_data.pwm_right = pwm;
      control_data.position = position;
      control_data.timestamp = xTaskGetTickCount();
      
      xQueueSend(Queue_Control, &control_data, portMAX_DELAY);
      xTaskNotifyGive(TaskHandle_Comm);
    }
    
    vTaskDelay(pdMS_TO_TICKS(10));
  }
}

static void Task_MotorStatus(void *argument) {
  MotorStatusData_t motor_status;
  static int32_t last_encoder_left = 0;
  static int32_t last_encoder_right = 0;
  uint8_t motor_status_flag = MOTOR_STATUS_NORMAL;
  
  while(1) {
    motor_status.encoder_left = __HAL_TIM_GET_COUNTER(&htim2);
    motor_status.encoder_right = __HAL_TIM_GET_COUNTER(&htim4);
    
    motor_status.speed_left = (float)(motor_status.encoder_left - last_encoder_left) * 0.1;
    motor_status.speed_right = (float)(motor_status.encoder_right - last_encoder_right) * 0.1;
    
    last_encoder_left = motor_status.encoder_left;
    last_encoder_right = motor_status.encoder_right;
    
    motor_status.current_left = 0;
    motor_status.current_right = 0;
    
    if(motor_status.speed_left > 200 || motor_status.speed_left < -200) {
      motor_status_flag |= MOTOR_STATUS_LEFT_OVER;
    } else {
      motor_status_flag &= ~MOTOR_STATUS_LEFT_OVER;
    }
    if(motor_status.speed_right > 200 || motor_status.speed_right < -200) {
      motor_status_flag |= MOTOR_STATUS_RIGHT_OVER;
    } else {
      motor_status_flag &= ~MOTOR_STATUS_RIGHT_OVER;
    }
    
    motor_status.status = motor_status_flag;
    motor_status.timestamp = xTaskGetTickCount();
    
    xQueueSend(Queue_MotorStatus, &motor_status, portMAX_DELAY);
    xTaskNotifyGive(TaskHandle_Bluetooth);
    
    vTaskDelay(pdMS_TO_TICKS(20));
  }
}

static void Task_Bluetooth(void *argument) {
  BluetoothData_t bt_data;
  ControlData_t control_data;
  MotorStatusData_t motor_status;
  uint8_t bt_rx_buffer[40];
  uint8_t bt_rx_index = 0;
  uint8_t bt_rx_len = 0;
  TickType_t bt_last_byte = 0;

  while(1) {
    if (bt_rx_index > 0 && (xTaskGetTickCount() - bt_last_byte) > pdMS_TO_TICKS(50)) {
      bt_rx_index = 0;
    }

    if(HAL_UART_Receive(&huart2, &bt_rx_buffer[bt_rx_index], 1, 10) == HAL_OK) {
      if(bt_rx_index >= sizeof(bt_rx_buffer)) {
        bt_rx_index = 0;
        continue;
      }

      if(bt_rx_index == 0 && bt_rx_buffer[0] != 0xBB) {
        continue;
      }

      if (bt_rx_index == 0) {
        bt_last_byte = xTaskGetTickCount();
        bt_rx_index = 1;
        continue;
      }

      if (bt_rx_len == 0) {
        bt_rx_len = bt_rx_buffer[1];
        if (bt_rx_len > 32) {
          bt_rx_index = 0;
          bt_rx_len = 0;
          continue;
        }
      }

      bt_last_byte = xTaskGetTickCount();
      bt_rx_index++;

      if(bt_rx_index >= bt_rx_len + 3 && bt_rx_buffer[bt_rx_index-1] == 0xCC) {
        bt_data.cmd = bt_rx_buffer[2];
        bt_data.len = bt_rx_len;
        for(int i=0; i<bt_rx_len && i<32; i++) {
          bt_data.data[i] = bt_rx_buffer[3 + i];
        }
        bt_data.timestamp = xTaskGetTickCount();

        switch(bt_data.cmd) {
          case BT_CMD_SET_SPEED: {
            float speed = (int16_t)(bt_data.data[0] << 8 | bt_data.data[1]) / 100.0f;
            xSemaphoreTake(RemoteControl_Mutex, portMAX_DELAY);
            RemoteControl.target_speed = speed;
            xSemaphoreGive(RemoteControl_Mutex);
            break;
          }
          case BT_CMD_SET_ANGLE: {
            float angle = (int16_t)(bt_data.data[0] << 8 | bt_data.data[1]) / 100.0f;
            xSemaphoreTake(RemoteControl_Mutex, portMAX_DELAY);
            RemoteControl.target_angle = angle;
            xSemaphoreGive(RemoteControl_Mutex);
            break;
          }
          case BT_CMD_SET_POSITION: {
            float pos = (int16_t)(bt_data.data[0] << 8 | bt_data.data[1]) / 10.0f;
            xSemaphoreTake(RemoteControl_Mutex, portMAX_DELAY);
            RemoteControl.target_position = pos;
            xSemaphoreGive(RemoteControl_Mutex);
            break;
          }
          case BT_CMD_STOP: {
            xSemaphoreTake(RemoteControl_Mutex, portMAX_DELAY);
            RemoteControl.target_speed = 0.0f;
            RemoteControl.target_angle = 0.0f;
            RemoteControl.target_position = 0.0f;
            xSemaphoreGive(RemoteControl_Mutex);
            break;
          }
          case BT_CMD_GET_STATUS: {
            if(xQueueReceive(Queue_Control, &control_data, 0) == pdTRUE) {
              uint8_t tx_data[9] = {0xBB, 0x05, 0x01,
                                     (uint8_t)(control_data.pitch * 10),
                                     (uint8_t)((int16_t)control_data.target_speed),
                                     (uint8_t)((int16_t)control_data.position),
                                     (uint8_t)((int16_t)control_data.pwm_left >> 8),
                                     (uint8_t)((int16_t)control_data.pwm_left & 0xFF),
                                     0xCC};
              HAL_UART_Transmit(&huart2, tx_data, 9, 20);
            }
            break;
          }
          case BT_CMD_SET_PID: {
            break;
          }
        }

        uint8_t ack[3] = {0xBB, 0x00, 0xCC};
        HAL_UART_Transmit(&huart2, ack, 3, 20);

        bt_rx_index = 0;
        bt_rx_len = 0;
      }
    }

    if(ulTaskNotifyTake(pdTRUE, 0)) {
      if(xQueueReceive(Queue_MotorStatus, &motor_status, 0) == pdTRUE) {
        uint8_t tx_data[11] = {0xBB, 0x08, 0x01,
                               (uint8_t)((int8_t)motor_status.speed_left),
                               (uint8_t)((int8_t)motor_status.speed_right),
                               (uint8_t)(motor_status.current_left >> 8),
                               (uint8_t)(motor_status.current_left & 0xFF),
                               (uint8_t)(motor_status.current_right >> 8),
                               (uint8_t)(motor_status.current_right & 0xFF),
                               motor_status.status, 0xCC};
        HAL_UART_Transmit(&huart2, tx_data, 11, 20);
      }
    }

    vTaskDelay(pdMS_TO_TICKS(10));
  }
}

static void Task_Comm(void *argument) {
  ControlData_t control_data;
  uint8_t uart_rx_buffer[16];
  uint8_t uart_rx_index = 0;

  while(1) {
    if(ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(5))) {
      if(xQueueReceive(Queue_Control, &control_data, 0) == pdTRUE) {
        uint8_t tx_data[12] = {0xAA, 0x01,
                               (uint8_t)((int8_t)(control_data.pitch * 10)),
                               (uint8_t)((int8_t)control_data.target_speed),
                               (uint8_t)((int16_t)control_data.position >> 8),
                               (uint8_t)((int16_t)control_data.position & 0xFF),
                               (uint8_t)((int16_t)control_data.pwm_left >> 8),
                               (uint8_t)((int16_t)control_data.pwm_left & 0xFF),
                               0x55};
        HAL_UART_Transmit(&huart1, tx_data, 9, 20);
      }
    }

    if(uart_rx_index >= sizeof(uart_rx_buffer)) {
      uart_rx_index = 0;
    }

    if(HAL_UART_Receive(&huart1, &uart_rx_buffer[uart_rx_index], 1, 0) == HAL_OK) {
      uart_rx_index++;
      if(uart_rx_index >= 3 && uart_rx_buffer[0] == 0xAA && uart_rx_buffer[uart_rx_index-1] == 0x55) {
        xSemaphoreTake(RemoteControl_Mutex, portMAX_DELAY);
        switch(uart_rx_buffer[1]) {
          case 0x01: {
            float speed = (int16_t)(uart_rx_buffer[2] << 8 | uart_rx_buffer[3]) / 100.0f;
            RemoteControl.target_speed = speed;
            break;
          }
          case 0x02: {
            float angle = (int16_t)(uart_rx_buffer[2] << 8 | uart_rx_buffer[3]) / 100.0f;
            RemoteControl.target_angle = angle;
            break;
          }
        }
        xSemaphoreGive(RemoteControl_Mutex);
        uart_rx_index = 0;
      }
    }
  }
}

void vApplicationMallocFailedHook(void) {
  Error_Handler();
}

void vApplicationStackOverflowHook(TaskHandle_t pxTask, char *pcTaskName) {
  (void)pxTask;
  (void)pcTaskName;
  Error_Handler();
}

void vApplicationIdleHook(void) {
  HAL_IWDG_Refresh(&hiwdg);
  __WFI();
}

void vApplicationTickHook(void) {
}
