
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
String writeMsgPathA = "system_logA.txt";
String writeMsgPathB = "system_logB.txt";
String writeMsgPathC = "system_logC.txt";
String writeMsgPathD = "system_logD.txt";
String msgA;
String msgB;
String msgC;
String msgD;
bool replaceFlag = false;
bool timeStampFlag = false;
String createFolderPath;
String createFilePath;


// bool isInCritical = false;
SemaphoreHandle_t xSemaphore = xSemaphoreCreateMutex();
// SemaphoreHandle_t xSemaphoreOpen = xSemaphoreCreateMutex();
// SemaphoreHandle_t xSemaphoreWrite = xSemaphoreCreateMutex();

// portMUX_TYPE writeFile_mutex = portMUX_INITIALIZER_UNLOCKED;

void writeMsgToPath(String path, String msg, bool replace = false, bool timeStamp = true){
  // taskENTER_CRITICAL(&writeFile_mutex);
  // isInCritical = !isInCritical;
  
  FsFile tempfile;
  Serial.println("write file, addr : " + String( (int)&tempfile ));

  if(replace){
    
    int isOpened;
    if(xSemaphoreTake( xSemaphore, portMAX_DELAY ) == pdTRUE){
      Serial.println("open");
      isOpened = tempfile.open(path.c_str(), O_WRONLY | O_CREAT);
    }
    xSemaphoreGive( xSemaphore );

    if (!isOpened) {     // open need char array, not string. So use c_str to convert
      Serial.println(" --> open " + path + " failed");
      showErrorLed();
      while(1){
        delay(1000);
      }
    }else{
      if(xSemaphoreTake( xSemaphore, portMAX_DELAY ) == pdTRUE){
        Serial.println("write");
        tempfile.println(msg.c_str());
        tempfile.close();
      }
      xSemaphoreGive( xSemaphore );
    }
  }else{
    int isOpened;
    if(xSemaphoreTake( xSemaphore, portMAX_DELAY ) == pdTRUE){
      Serial.println("open");
      isOpened = tempfile.open(path.c_str(), O_WRONLY | O_CREAT | O_APPEND);
    }
    xSemaphoreGive( xSemaphore );
    if (!isOpened) {     // open need char array, not string. So use c_str to convert
      Serial.println(" --> open " + path + " failed");
      showErrorLed();
      while(1){
        delay(1000);
      }
    }else{
      if(xSemaphoreTake( xSemaphore, portMAX_DELAY ) == pdTRUE){
        Serial.println("write");
        tempfile.println(msg.c_str());
        tempfile.close();
      }
      xSemaphoreGive( xSemaphore );
    }
  }
  Serial.println("finished");
}

TaskHandle_t t0;
TaskHandle_t t1;
TaskHandle_t t2;
TaskHandle_t t3;

TaskHandle_t t4;
TaskHandle_t t5;
TaskHandle_t t6;
TaskHandle_t t7;

bool needWriteMSGToPathA = false;
bool needWriteMSGToPathB = false;
bool needWriteMSGToPathC = false;
bool needWriteMSGToPathD = false;

void checkNeedWriteMSGToPathA(void* pvParameters){
  Serial.println("checkNeedWriteMSGToPathA : created");
  while(1){
    if(needWriteMSGToPathA){
      Serial.println("needWriteMSGToPathA");
      needWriteMSGToPathA = !needWriteMSGToPathA;
      writeMsgToPath(writeMsgPathA, msgA, replaceFlag, timeStampFlag);
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
      writeMsgToPath(writeMsgPathB, msgB, replaceFlag, timeStampFlag);
    }
    vTaskDelay(500);
  }
}

void checkNeedWriteMSGToPathC(void* pvParameters){
  Serial.println("checkNeedWriteMSGToPathC : created");
  while(1){
    if(needWriteMSGToPathC){
      Serial.println("needWriteMSGToPathC");
      needWriteMSGToPathC = !needWriteMSGToPathC;
      writeMsgToPath(writeMsgPathC, msgC, replaceFlag, timeStampFlag);
    }
    vTaskDelay(25);
  }
}

void checkNeedWriteMSGToPathD(void* pvParameters){
  Serial.println("checkNeedWriteMSGToPathD : created");
  while(1){
    if(needWriteMSGToPathD){
      Serial.println("needWriteMSGToPathD");
      needWriteMSGToPathD = !needWriteMSGToPathD;
      writeMsgToPath(writeMsgPathD, msgD, replaceFlag, timeStampFlag);
    }
    vTaskDelay(40);
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

void createRequestC(void* pvParameters){
  Serial.println("createRequestC : created");
  while(1){
    if(!needWriteMSGToPathC){
      needWriteMSGToPathC = !needWriteMSGToPathC;
      msgC = "request_MSG_C";
    }
    vTaskDelay(50);
  }
}

void createRequestD(void* pvParameters){
  Serial.println("createRequestD : created");
  while(1){
    if(!needWriteMSGToPathD){
      needWriteMSGToPathD = !needWriteMSGToPathD;
      msgD = "request_MSG_D";
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

    xTaskCreatePinnedToCore(
    checkNeedWriteMSGToPathC,
    "checkNeedWriteMSGToPathC",
    4096,                                   /* Stack size of task */
    NULL,                                   /* parameter of the task */
    2,                                      /* priority of the task */    // 0 : idel, I2S : 2
    &t4,                                 /* task handle */
    OTHER_TASK_CPU                          /* CPU core */
  );

  xTaskCreatePinnedToCore(
    checkNeedWriteMSGToPathD,
    "checkNeedWriteMSGToPathD",
    4096,                                   /* Stack size of task */
    NULL,                                   /* parameter of the task */
    2,                                      /* priority of the task */    // 0 : idel, I2S : 2
    &t5,                                 /* task handle */
    OTHER_TASK_CPU                          /* CPU core */
  );

  xTaskCreatePinnedToCore(
    createRequestC,                    /* Task function. */
    "createRequestC",           /* name of task. */
    4096,                                   /* Stack size of task */
    NULL,                                   /* parameter of the task */
    2,                                      /* priority of the task */
    &t6,                        /* task handle */
    OTHER_TASK_CPU                          /* CPU core */
  );

  xTaskCreatePinnedToCore(
    createRequestD,                    /* Task function. */
    "createRequestD",           /* name of task. */
    4096,                                   /* Stack size of task */
    NULL,                                   /* parameter of the task */
    2,                                      /* priority of the task */
    &t7,                        /* task handle */
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

