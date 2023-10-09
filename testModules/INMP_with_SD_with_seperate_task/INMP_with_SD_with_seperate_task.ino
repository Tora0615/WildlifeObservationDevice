#include "setting.h"
#include "myINMP441.h"

bool isNeedToRecord = false;
TaskHandle_t tINMP;
char channel_tag = 'B';

void recordSound(){
  
  // calculate filename
  String recordPath = "test.wav";
  char filename[40];
  strcpy(filename, recordPath.c_str());

  #ifdef RECORD_SOUND_DEBUG
    Serial.println(recordPath);
  #endif

  // recognize channel tag
  if(channel_tag == 'B'){
    recordWithDualChannel(10, filename, 1.0);
  }else if(channel_tag == 'L'){
    recordWithMonoChannel(10, filename, 1.0, CHANNEL_LEFT);
  }else if(channel_tag == 'R'){
    recordWithMonoChannel(10, filename, 1.0, CHANNEL_RIGHT);
  }

  // avoid infinite loop
  // runner.deleteTask(t_recordSound);  --> Now we don't use task scheduler to handle this.
}

void checkIfNeedToRecord(void* pvParameters){   // void* pvParameters : don't accept any value
  Serial.println("checkIfNeedToRecord : created");
  while(1){
    vTaskDelay(10 / portTICK_PERIOD_MS);
    if(isNeedToRecord){
      isNeedToRecord = !isNeedToRecord;
      recordSound();
    }

  }
}

void SDInit(){
  if(!sd.begin(CHIP_SELECT, SPI_SPEED)){
    Serial.println("SD card init error : NO SD card");
    while(1){
      delay(1000);
    }
  }

  // show
  #ifdef SD_WRITE_MSG_DEBUG
    Serial.println("SPI_SPEED : " + String(SPI_SPEED));
  #endif
}

void createRTOSTasks() {
  Serial.println("RTOS : createCoreTasks");

  xTaskCreatePinnedToCore(
    checkIfNeedToRecord,
    "INMPThreadAtCore0",
    20480,                                  /* Stack size of task */
    NULL,                                   /* parameter of the task */
    2,                                      /* priority of the task */    
    &tINMP,                                 /* task handle */
    INMP_CPU                                /* CPU core */
  );

  xTaskCreatePinnedToCore(
    transmitToSD,                    /* Task function. */
    "transmitToSD",                  /* name of task. */
    20480,                                   /* Stack size of task */
    NULL,                                   /* parameter of the task */
    2,                                      /* priority of the task */
    &tTransmitHandle,                         /* task handle */
    OTHER_TASK_CPU                          /* CPU core */
  );
}



void setup() {
  Serial.begin(115200);  

  // system basic part init 
  SDInit();
  createRTOSTasks();
  isNeedToRecord = true;
}
 
void loop() {}
