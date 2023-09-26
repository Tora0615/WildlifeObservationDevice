
#include <SPI.h>
#include "SdFat.h"
#include "sdios.h"
#define SPI_SPEED SD_SCK_MHZ(4)
#define CHIP_SELECT 5
const int8_t DISABLE_CHIP_SELECT = -1;


void showErrorLed(){
  digitalWrite(16, HIGH); 
}


// global file pointer / SD_FAT_TYPE 3 (FAT16/FAT32 and exFAT)
SdFs sd;
FsFile systemLog;
FsFile sensorData;

void SDInit(){
  if (!sd.begin(CHIP_SELECT, SPI_SPEED)) {
    Serial.println("SD card init error");
    while(1){
      delay(1000);
    }
  }
}


String writeMsgPath = "system_log.txt";
String msgA;
String msgB;
bool replaceFlag = false;
bool timeStampFlag = false;
String createFolderPath;
String createFilePath;


// bool isInCritical = false;
SemaphoreHandle_t xSemaphore = xSemaphoreCreateMutex();
portMUX_TYPE writeFile_mutex = portMUX_INITIALIZER_UNLOCKED;

void writeMsgToPath(String path, String msg, bool replace = false, bool timeStamp = true){
  // taskENTER_CRITICAL(&writeFile_mutex);
  // isInCritical = !isInCritical;
  Serial.println("write file");

  // wait for release
  if(xSemaphoreTake( xSemaphore, portMAX_DELAY ) == pdTRUE){
    FsFile tempfile;
    
    if(replace){
      
      int isOpened = tempfile.open(path.c_str(), O_WRONLY | O_CREAT);

      if (!isOpened) {     // open need char array, not string. So use c_str to convert
        Serial.println(" --> open " + path + " failed");
        showErrorLed();
        while(1){
          delay(1000);
        }
      }else{
        tempfile.println(msg.c_str());
      }
    }else{

      int isOpened = tempfile.open(path.c_str(), O_WRONLY | O_CREAT | O_APPEND);

      if (!isOpened) {     // open need char array, not string. So use c_str to convert
        Serial.println(" --> open " + path + " failed");
        showErrorLed();
        while(1){
          delay(1000);
        }
      }else{

        tempfile.println(msg.c_str());
        
      }
    }

    // isInCritical = !isInCritical;
    // taskEXIT_CRITICAL(&writeFile_mutex);

  }

  xSemaphoreGive( xSemaphore );
  Serial.println("finished");
}

TaskHandle_t t0;
TaskHandle_t t1;
TaskHandle_t t2;
TaskHandle_t t3;

bool needWriteMSGToPathA = false;
bool needWriteMSGToPathB = false;


void checkNeedWriteMSGToPathA(void* pvParameters){
  Serial.println("checkNeedWriteMSGToPathA : created");
  while(1){
    if(needWriteMSGToPathA){
      Serial.println("needWriteMSGToPathA");
      needWriteMSGToPathA = !needWriteMSGToPathA;
      writeMsgToPath(writeMsgPath, msgA, replaceFlag, timeStampFlag);
    }
    vTaskDelay(50);
  }
}

void checkNeedWriteMSGToPathB(void* pvParameters){
  Serial.println("checkNeedWriteMSGToPathB : created");
  while(1){
    if(needWriteMSGToPathB){
      Serial.println("needWriteMSGToPathB");
      needWriteMSGToPathB = !needWriteMSGToPathB;
      writeMsgToPath(writeMsgPath, msgB, replaceFlag, timeStampFlag);
    }
    vTaskDelay(500);
  }
}


void createRequestA(void* pvParameters){
  Serial.println("createRequestA : created");
  while(1){
    if(!needWriteMSGToPathA){
      needWriteMSGToPathA = !needWriteMSGToPathA;
      msgA = "request_MSG_A";
    }
    vTaskDelay(50);
  }
}

void createRequestB(void* pvParameters){
  Serial.println("createRequestB : created");
  while(1){
    if(!needWriteMSGToPathB){
      needWriteMSGToPathB = !needWriteMSGToPathB;
      msgB = "request_MSG_B";
    }
    vTaskDelay(50);
  }
}

#define OTHER_TASK_CPU 1
void createRTOSTasks() {
  Serial.println("RTOS : createCoreTasks");

  xTaskCreatePinnedToCore(
    checkNeedWriteMSGToPathA,
    "checkNeedWriteMSGToPathA",
    4096,                                   /* Stack size of task */
    NULL,                                   /* parameter of the task */
    2,                                      /* priority of the task */    // 0 : idel, I2S : 2
    &t0,                                 /* task handle */
    OTHER_TASK_CPU                          /* CPU core */
  );

  xTaskCreatePinnedToCore(
    checkNeedWriteMSGToPathB,
    "checkNeedWriteMSGToPathB",
    4096,                                   /* Stack size of task */
    NULL,                                   /* parameter of the task */
    2,                                      /* priority of the task */    // 0 : idel, I2S : 2
    &t1,                                 /* task handle */
    OTHER_TASK_CPU                          /* CPU core */
  );

  xTaskCreatePinnedToCore(
    createRequestA,                    /* Task function. */
    "createRequestA",           /* name of task. */
    4096,                                   /* Stack size of task */
    NULL,                                   /* parameter of the task */
    2,                                      /* priority of the task */
    &t2,                        /* task handle */
    OTHER_TASK_CPU                          /* CPU core */
  );

  xTaskCreatePinnedToCore(
    createRequestB,                    /* Task function. */
    "createRequestB",           /* name of task. */
    4096,                                   /* Stack size of task */
    NULL,                                   /* parameter of the task */
    2,                                      /* priority of the task */
    &t3,                        /* task handle */
    OTHER_TASK_CPU                          /* CPU core */
  );
}


void setup() {
  Serial.begin(115200);
  SDInit();
  createRTOSTasks();
}


void loop(){

}

