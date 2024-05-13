#include "HardwareSerial.h"
#include <stdint.h>
/*---- include guard ----*/
#ifndef TASKFILEREADER_H
#define TASKFILEREADER_H

/*---- macro or define ----*/

/*---- official lib ----*/ 

/*---- other involve lib  ----*/
#include "utills.h"   // include sd_operation / rtc_timer / setting



/*---- classes, variables or function define  ----*/
/* new task structure*/
// Our task is minuate based --> 24hr == 1440min --> if use char : only 1440 byte
// We can simply fill the array by task like ["0", "1", "2", "0", ... ,"0"]. Every slot is a min
// About the record setting, we can use another array to storage the value


/*---- classes, variables or function define  ----*/
// defined in setting.h
// taskScheduleList 
// readTask    --> the index of task array 
// readSetting --> the index of setting array 

// local variables
int recordSettingArrayMaxSize = 2;    // to know the malloc situation 
int recordSettingArrayUsedSize = 0;   // to know the malloc situation 
typedef struct recordSetting_t{
  int duration_time;
  char channel;
  float multiple;
}recordSetting;

recordSetting *recordSettingArray = (recordSetting*)malloc( sizeof(recordSetting) * recordSettingArrayMaxSize);

/*-------- function implement --------*/
int hour24ConvetToMin(int input){
  return (input/100)*60 + input %100 ;
}

int minConvertTohour24(int input){
  return (input/60)*100 + input % 60;
}

uint8_t taskTouint8t(char input){
  // A    B     C     D
  // 2^3  2^2   2^1   2^0
  if (input == 'A'){
    return 8;
  }else if (input == 'B') {
    return 4;
  }else if (input == 'C') {
    return 2;
  }else if (input == 'D') {
    return 1;
  }else{
    Serial.print("Unknow task");
    return 0;
  }
}

String uint8ToString(uint8_t input){
  uint8_t A = 0;
  uint8_t B = 0;
  uint8_t C = 0;
  uint8_t D = 0;
  A = input / 8;
  input = input % 8;
  B = input / 4;
  input = input % 4;
  C = input / 2;
  input = input % 2;
  D = input;
  String out = "";
  if (A) out += 'A';
  if (B) out += 'B';
  if (C) out += 'C';
  if (D) out += 'D';
  return out;
}

/* add a pointer to a list end*/
void addOneSetting(recordSetting **pointerToTaskArray, recordSetting *pointerToTask, int *pointerToArrayMaxSize, int *pointerToArrayUsedIndex){  
  // if array size is not enough
  if(*pointerToArrayUsedIndex == *pointerToArrayMaxSize){
    // create a temp array with double size
    recordSetting *tempTaskArray = (recordSetting*)malloc( sizeof(recordSetting) * *pointerToArrayMaxSize * 2);
    // copy data to new array
    memcpy(tempTaskArray, *pointerToTaskArray, *pointerToArrayMaxSize * sizeof(recordSetting) );
    // release old array ram space
    free(*pointerToTaskArray);
    // new array pointer give to old name
    *pointerToTaskArray = tempTaskArray;
    // double variable
    *pointerToArrayMaxSize = *pointerToArrayMaxSize * 2;
  }

  // give value to that array slot (copy value, not copy address)
  *(*pointerToTaskArray + *pointerToArrayUsedIndex) = *pointerToTask;
  // index + 1
  *pointerToArrayUsedIndex += 1;
}

void addTask(String input){
  // write log
  // writeMsgToPath(systemLogPath, "|-- Start to parse task command : " + input);

  //// init temp variable
  int index = 0;   // for count handled to which index
  char *temp[5];   // a pointer can storage 5 char array, meaning : [start_time, task, duration/period, L/R(if have), ratial(if have)]

  //// parse that line of input (split by ",")
  temp[index] = strtok((char *)input.c_str(), ",");   // cut the input by "," and return the first cut part
  while( temp[index] != NULL){                        // if still can split, keep running
    #ifdef PARSE_TASK_DEBUG
      Serial.print(String(temp[index]) + " / ");
    #endif
    index += 1;
    temp[index] = strtok(NULL, ",");                  // keep split from the last position by passing NULL into it.
  }
  #ifdef PARSE_TASK_DEBUG
    Serial.println("");
  #endif

  //// see the len of the task, then add it to the task array
  ////// len == 3 --> repeat work / len == 5 --> record
  if(index == 3){
    ////// repeat add to the task array
    //////// convert data type
    int start_min_of_a_day = hour24ConvetToMin(atoi(temp[0]));
    char task = temp[1][0];
    int interval = atoi(temp[2]);
    //////// repeat mark the task array
    int toAddMin = start_min_of_a_day;
    while(toAddMin < MIN_A_DAY){
      taskScheduleList[toAddMin] += taskTouint8t(task);
      toAddMin += interval;
    }
  }else{
    ////// only need to add once, but need to add additional setting array
    //////// convert data type
    int start_min_of_a_day = hour24ConvetToMin(atoi(temp[0]));
    char task = temp[1][0];
    int duration = atoi(temp[2]);
    char channel = temp[3][0];
    float multiple = atof(temp[4]);
    //////// mark the task array once
    taskScheduleList[start_min_of_a_day] += taskTouint8t(task);
    //////// add a setting to array
    ////////// new a struct and give value
    recordSetting oneTempSetting;
    oneTempSetting.duration_time = duration;
    oneTempSetting.channel = channel;
    oneTempSetting.multiple = multiple;
    ////////// attach to the malloc array end
    addOneSetting(&recordSettingArray, &oneTempSetting, &recordSettingArrayMaxSize, &recordSettingArrayUsedSize);
  }
}


void checkScheduleFileExist(){
  if (!sd.exists(SCHEDULE_FILE.c_str())){
    // print error
    Serial.println("Init failed! Don't have file : " + SCHEDULE_FILE );
    // write log
    writeMsgToPath(systemLogPath, "Init failed! Don't have file : " + SCHEDULE_FILE + ". Please see exampleSchedule.txt");
    
    // write example file
    writeMsgToPath("example_schedule.txt", 
      "0000,A,900,B,1.5\n"
      "0030,A,900,B,1\n"
      "0200,A,900,R,1.5\n"
      "0230,A,900,R,1\n"
      "0400,A,900,R,1.5\n"
      "0430,A,900,R,1\n"
      "1600,A,900,R,1.5\n"
      "1630,A,900,R,1\n"
      "1800,A,900,R,1.5\n"
      "1830,A,900,R,1\n"
      "2000,A,900,B,1.5\n"
      "2030,A,900,B,1\n"
      "2200,A,900,R,1.5\n"
      "2230,A,900,R,1\n"
      "0000,B,10\n"
      "0000,C,10\n"
      "0000,D,10\n"
      "#---------\n"
      "任務代碼 : \n"
      "A : Sound (INMP441)\n"
      "B : temperature & moisture (DHT22)\n"
      "C : temperature (DS18B20)\n"
      "D : Battery voltage\n"
      "\n"
      "參數說明 : \n"
      "任務 A\n"
      "初始時間 (24小時制,無標點,不可有小數點), 任務代碼, 執行時間(sec,不可有小數點), L/R/B (聲道左/右/兩者), 音量幾倍 (基準為1，建議範圍 : 0.5 ~ 2倍)\n"
      "任務 BCD\n"
      "初始時間 (24小時制,無標點,不可有小數點), 任務代碼, 執行間隔(min,不可有小數點)\n"
      "\n"
      "其他 : \n"
      "請將 example_schedule.txt 重新命名成 schedule.txt，程式才能正確執行\n"
      "\n"
      "常用換算 : \n"
      "01 min = 60 sec\n"
      "05 min = 300 sec\n"
      "10 min = 600 sec\n"
      "15 min = 900 sec\n"
      "30 min = 1800 sec\n"
      "45 min = 2700 sec\n"
      "60 min = 3600 sec\n"
    ,"", true, false);   // custem timestamp msg, append, timestamp

    // delay and show light
    showNoScheduleFileLED();
  }
  // Write log
  writeMsgToPath(systemLogPath, "ScheduleFile found!");
}

void addAllTaskFromFile(){
  // 0. Write log
  writeMsgToPath(systemLogPath, "Start to add all tasks");

  // 1. open file 
  #ifdef SD_USE_NORMAL
    FsFile taskFile;
  #else
    ExFile taskFile;
  #endif 
  if (!taskFile.open(SCHEDULE_FILE.c_str(), FILE_READ)) {
    Serial.println("open failed");
  }

  // 2. read file until EOF
  //// This is a line's max
  const int lenOfLine = 40;
  while (taskFile.available()) {
    ////// init a buffer for save a line
    char buffer[lenOfLine];                   // create buffer
    memset(buffer, 0, sizeof(buffer));        // clean the buffer 

    ////// save a line
    //////// save char one by one untill meet '\n' ('\n' will be ignore)
    int index = 0;
    while(1){
      char tempChar = taskFile.read();        // read a word, then save to buffer 
      if(tempChar == '\n'){
        break;
      }
      buffer[index] = tempChar;
      index += 1;
    }
    buffer[index] = '\0';                     // add postfix

    ////// info 
    #ifdef ADD_ALL_TASK_FROM_FILE_DEBUG
      Serial.println(String(buffer) + " : " + String(index));
    #endif

    ////// Check tnd loop condition of this line
    if(String(buffer) == "#---------"){           // 9 x '-'
      break;
    }else if(String(buffer) == "#----------"){    // 10 x '-'
      isEvaluation = false;
      break;
    }

    ////// if not last line, add task by that line
    addTask(String(buffer));
  }
  //// read finished, close file
  taskFile.close();
  //// info 
  #ifdef ADD_ALL_TASK_FROM_FILE_DEBUG
    Serial.println("Done");
  #endif

  // Write log
  writeMsgToPath(systemLogPath, "Add all tasks successful!");
}

// To find the matchd index to the system time
void findTheMatchedArrayReadIndex(){

}

void printAllTask(){
  Serial.println("printAllTask -- start");
  int settingIndex = 0;
  for (int i=0; i<MIN_A_DAY ;i++){
    // Have actual task 
    if (taskScheduleList[i] != 0){
      Serial.println("Index : " + String(i) + ", task : " + String(taskScheduleList[i]) + "(" + uint8ToString(taskScheduleList[i]) + ")");
      // if have record task
      if (taskScheduleList[i] > 7){
        Serial.println("-- duration_time : " + String(recordSettingArray[settingIndex].duration_time));
        Serial.println("-- channel : " + String(recordSettingArray[settingIndex].channel));
        Serial.println("-- multiple : " + String(recordSettingArray[settingIndex].multiple));
        settingIndex+=1;
      }
    }
  }
  Serial.println("printAllTask -- end");
}

#endif