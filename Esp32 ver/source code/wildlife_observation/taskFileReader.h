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
// defined in "setting.h"
// taskScheduleList --> the define of when to do what task
// readTaskIndex    --> the index of task array 
// readSettingIndex --> the index of setting array 
// int recordSettingArrayMaxSize = 2;    // to know the malloc situation 
// int recordSettingArrayUsedSize = 0;   // to know the malloc situation 
// recordSetting struct 
// recordSetting *recordSettingArray = (recordSetting*)malloc( sizeof(recordSetting) * recordSettingArrayMaxSize);

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

// return the pointer of task array 
#define uzipTaskLength 4
#define TASK_A 0
#define TASK_B 1
#define TASK_C 2
#define TASK_D 3
uint8_t* uint8NumToCharArr(uint8_t input){
  // [A,B,C,D]
  uint8_t *taskArray = (uint8_t*)malloc(sizeof(uint8_t)*4);
  // A
  taskArray[TASK_A] = input / 8;
  input = input % 8;
  // B
  taskArray[TASK_B]  = input / 4;
  input = input % 4;
  // C
  taskArray[TASK_C]  = input / 2;
  input = input % 2;
  // D
  taskArray[TASK_D]  = input;
  return taskArray;
}

String uint8ToString(uint8_t input){
  
  // input is from 0 to 15
  // the four bit is : A, B, C, D (2^3, 2^2, 2^1, 2^0)
  // We want to get a task string like "ABCD" if the bit is 1
  // e.g : 15 --> 1111 --> ABCD
  // e.g : 10 --> 1010 --> AC
  // e.g :  5 --> 0101 --> BD

  String rtn = "";
  char tasks[] = {'D', 'C', 'B', 'A'};
  for (int i = 0; i < 4; ++i) {  
      // i++ and ++i is the same in this case, but in compiler level, ++i is faster
      // input compare with the bit mask, 1<<i is 2^i
      if (input & (1 << i)) {
        // this can make the order noemal
        rtn = String(tasks[i]) + rtn;
      }
  }
  if (input == 0){
    return String("N/A");
  }
  return rtn;
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
  myFileFormat taskFile;

  if (!taskFile.open(SCHEDULE_FILE.c_str(), O_RDONLY)) {  // read only 
    Serial.println("open failed");
  }

  ////// info - list all items in empty array (check array clean or not)
  #ifdef ADD_ALL_TASK_FROM_FILE_DEBUG
    Serial.println("-- before adding tasks -- check array clean or not --");
    for (int i=0; i<24; i++){
      for (int j=0; j< 60; j++){
        Serial.print(String(taskScheduleList[i*60+j]) + "\t");
      }
      Serial.println("");
    }
  #endif

  // 2. read file until EOF
  //// info 
  #ifdef ADD_ALL_TASK_FROM_FILE_DEBUG
    Serial.println("\nStart to print lines we read : ");
  #endif
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
    Serial.println("Add tasks done\n");
  #endif

  ////// info - list all items in fulled array (check array items)
  #ifdef ADD_ALL_TASK_FROM_FILE_DEBUG
    Serial.println("-- after adding tasks -- check array items --");
    for (int i=0; i<24; i++){
      for (int j=0; j< 60; j++){
        Serial.print(String(taskScheduleList[i*60+j]) + "\t");
      }
      Serial.println("");
    }
  #endif

  // Write log
  writeMsgToPath(systemLogPath, "Add all tasks successful!");
}

// TODO, maybe can delete it ?
// To find the matchd index to the system time
// Then we can execute future tasks by this basic shift
void findTheIndexOfCurrentTime(){
  // compair time and save to "readTaskIndex" in "setting.h"
  // "readTaskIndex" is the index of the task "GOING TO RUN"
  // taskScheduleList is a array in MIN_A_DAY(1440) size 

  // <60 sec == run at the moment of the min 0 change to 1
  // !! this is the index of current time !! 
  readTaskIndex = getPassedSecOfToday()/60 + 1;  
  // 23:59:00 ~ 23:59:59 == 1439min00sec~59sec --> readTaskIndex == 1440, but out of range
  // so it need to re-zero
  if (readTaskIndex==MIN_A_DAY){
    readTaskIndex = 0;
    isCrossDay = true;
  }
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