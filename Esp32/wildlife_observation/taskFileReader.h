#ifndef TASKFILEREADER_H
#define TASKFILEREADER_H

#include "sd_operation.h"
#include "rtc_timer.h"

// debug setting part
// #define PARSE_TASK_DEBUG
// #define ADD_REPEAT_WORKS_DEBUG
// #define SORT_TASK_DEBUG
#define ADD_ALL_TASK_FROM_FILE_DEBUG


/* process task command */
int arrayMaxSize = 2;      // task array max 
int arrayUsedIndex = 0;    // task array used
int arrayReadIndex = 0;    // read position, re-zero when day change

typedef struct complexTask_t{
  int start_min_of_a_day;
  char task;
  float time;
  int channel;
  float multiple;
}complexTask;

typedef struct simpleTask_t{
  int start_min_of_a_day;
  char task;
}simpleTask;

typedef struct myTask_t {
  byte setType;  // 0 for simple, 1 for complex
  union {
    simpleTask simple;
    complexTask complex;
  } taskType;
}myTask;

myTask *taskArray = (myTask*)malloc( sizeof(myTask) * arrayMaxSize );

void addTask(myTask **pointerToTaskArray, myTask *pointerToTask, int *pointerToArrayMaxSize, int *pointerToArrayUsedIndex){
  // if array size is not enough
  if(*pointerToArrayUsedIndex == *pointerToArrayMaxSize){
    // create a temp array with double size
    myTask *tempTaskArray = (myTask*)malloc( sizeof(myTask) * *pointerToArrayMaxSize * 2);
    // copy data to new array
    memcpy(tempTaskArray, *pointerToTaskArray, *pointerToArrayMaxSize * sizeof(myTask) );
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




void sortTask(myTask *taskArray, int arrayUsedIndex){
  // bubbleSort
	int i, j;
  myTask temp;
	bool exchanged = true;
	
	for (i=0; exchanged && i<arrayUsedIndex-1; i++){ 
    exchanged = false;
		for (j=0; j<arrayUsedIndex-1-i; j++){ 
      // read value depend on struct type
      int a = taskArray[j].setType == 0 ? taskArray[j].taskType.simple.start_min_of_a_day : taskArray[j].taskType.complex.start_min_of_a_day;
      int b = taskArray[j+1].setType == 0 ? taskArray[j+1].taskType.simple.start_min_of_a_day : taskArray[j+1].taskType.complex.start_min_of_a_day;
      
      #ifdef SORT_TASK_DEBUG
        Serial.println(a);
        Serial.println(b);
      #endif

			if (a>b){ // if statement SHOULDEN'T seperate into two line
        #ifdef SORT_TASK_DEBUG
          Serial.println("swap");
        #endif
				temp = taskArray[j];
				taskArray[j] = taskArray[j+1];
				taskArray[j+1] = temp;
				exchanged = true; 
			}
    }
  }
}


int hour24ConvetToMin(int input){
  return (input/100)*60 + input %100 ;
}

int minConvertTohour24(int input){
  return (input/60)*100 + input % 60;
}

void printAllTask(myTask *taskArray, int inputArrayUsedIndex){
  for(int index = 0; index < inputArrayUsedIndex; index++){
    // Serial.println("Address : " + String( (int)(taskArray+index)) );
    if((taskArray+index)->setType == 0){  // simple task
      Serial.print("start_min_of_a_day : " + String( (taskArray+index)->taskType.simple.start_min_of_a_day ) + "(" + String( minConvertTohour24( (taskArray+index)->taskType.simple.start_min_of_a_day ) ) + ")");
      Serial.println(" / task : " + String( (taskArray+index)->taskType.simple.task ));
    }else{  // complex task
      Serial.print("start_min_of_a_day : " + String( (taskArray+index)->taskType.complex.start_min_of_a_day ) + "(" + String( minConvertTohour24((taskArray+index)->taskType.complex.start_min_of_a_day) ) + ")");
      Serial.print(" / task : " + String( (taskArray+index)->taskType.complex.task ));
      Serial.print(" / time : " + String( (taskArray+index)->taskType.complex.time ));
      Serial.print(" / channel : " + String( (taskArray+index)->taskType.complex.channel ));
      Serial.println(" / multiple : " + String( (taskArray+index)->taskType.complex.multiple ));
    }
  }
}

myTask parseTasks(String input){
  int lenCount = 0;
  char *temp[5];  // a pointer point to a value which can storage 5 char array pointer

  char *token;
  temp[lenCount] = strtok((char *)input.c_str(), ",");  // remove error "invalid conversion from 'const char*' to 'char*' ""
  while( temp[lenCount] != NULL){   // if still can parse, keep running
    #ifdef PARSE_TASK_DEBUG
      Serial.print(String(temp[lenCount]) + " / ");
    #endif
    lenCount += 1;
    temp[lenCount] = strtok(NULL, ",");
  }
  #ifdef PARSE_TASK_DEBUG
    Serial.println("");
  #endif

  myTask tempTask;
  tempTask.setType = 1;
  if(lenCount == 3){
    tempTask.taskType.complex.start_min_of_a_day = hour24ConvetToMin(atoi(temp[0]));
    tempTask.taskType.complex.task = temp[1][0];
    tempTask.taskType.complex.time = atof(temp[2]);
    tempTask.taskType.complex.channel = ' ';
    tempTask.taskType.complex.multiple = 0.0;
  }else{
    tempTask.taskType.complex.start_min_of_a_day = hour24ConvetToMin(atoi(temp[0]));
    tempTask.taskType.complex.task = temp[1][0];
    tempTask.taskType.complex.time = atof(temp[2]);
    tempTask.taskType.complex.channel = atoi(String(temp[3][0]).c_str());
    tempTask.taskType.complex.multiple = atof(temp[4]);
  }
  return tempTask;
}


void addRepeatWorks(myTask *inputTaskArray){
  // new variable and larger size array
  int tempArrayMaxSize = 2;
  int tempArrayUsedIndex = 0;
  myTask *temptaskArray = (myTask*)malloc( sizeof(myTask) * tempArrayMaxSize );

  for (int i=0; i<arrayUsedIndex; i++){
    if((inputTaskArray+i)->taskType.complex.task == 'A'){
      // direct copy 
      addTask(&temptaskArray, (inputTaskArray+i), &tempArrayMaxSize, &tempArrayUsedIndex);
      #ifdef ADD_REPEAT_WORKS_DEBUG
        Serial.println("direct copy" + String(i));
        Serial.println("tempArrayMaxSize : " + String(tempArrayMaxSize));
        Serial.println("tempArrayUsedIndex : " + String(tempArrayUsedIndex));
      #endif
    }else{
      // count repeat time
      int repeatTime = 24.0 / ((inputTaskArray+i)->taskType.complex.time);
      #ifdef ADD_REPEAT_WORKS_DEBUG
        Serial.println("repeatTime : " + String(repeatTime));
      #endif

      // repeat N times to add task to temp array
      for (int j=0; j<repeatTime; j++){
        myTask tempTask;        // !!same address, if malloc will be different!!
        tempTask.setType = 0;
        tempTask.taskType.simple.task = ( (inputTaskArray+i)->taskType.complex.task );
        tempTask.taskType.simple.start_min_of_a_day = ( (inputTaskArray+i)->taskType.complex.start_min_of_a_day) + j * ((inputTaskArray+i)->taskType.complex.time) * 60;
        addTask(&temptaskArray, &tempTask, &tempArrayMaxSize, &tempArrayUsedIndex);

        #ifdef ADD_REPEAT_WORKS_DEBUG
          Serial.println("loop create: " + String(j+1));
          Serial.println("tempArrayMaxSize : " + String(tempArrayMaxSize));
          Serial.println("tempArrayUsedIndex : " + String(tempArrayUsedIndex));
        #endif
      }
    }
  }

  // here can confirm change success or not
  // printAllTask(temptaskArray, tempArrayUsedIndex);

  // replace with new array
  arrayMaxSize = tempArrayMaxSize;
  arrayUsedIndex = tempArrayUsedIndex;
  taskArray = temptaskArray;   // local variable SHOULD NOT same as gloable, or replace will not be success
}



void checkScheduleFileExist(){
  if (!sd.exists(SCHEDULE_FILE.c_str())){
    // print error
    Serial.println("Init failed! Don't have file : " + SCHEDULE_FILE );
    writeMsgToPath(systemLogPath, "Init failed! Don't have file : " + SCHEDULE_FILE + ". Please see exampleSchedule.txt");
    
    // write example file
    writeMsgToPath("example_schedule.txt", 
      "0000,A,30,L,1\n"
      "0000,A,15,R,2\n"
      "0000,A,10,B,1\n"
      "0000,A,20,B,0.5\n"
      "0000,B,0.5\n"
      "0000,C,0.5\n"
      "0000,D,1\n"
      "#---------\n"
      "任務代碼 : \n"
      "A : Sound (INMP441)\n"
      "B : temperature & moisture (DHT22)\n"
      "C : temperature (DS18B20)\n"
      "D : Battery voltage\n"
      "\n"
      "參數說明 : \n"
      "任務 A\n"
      "初始時間, 任務代碼, 執行時間(min), L/R/B (聲道左/右/兩者), 音量幾倍(基準為1，建議範圍 : 0.5 ~ 2倍)\n"
      "任務 BCD\n"
      "初始時間, 任務代碼, 執行間隔(hr)\n"
      "\n"
      "其他 : \n"
      "請將 example_schedule.txt 重新命名成 schedule.txt，程式才能正確執行\n"
    , true);

    // delay 
    while(1) delay(10000);
  }
}

/* read info from file and process*/
void addAllTaskFromFile(){
  // open file 
  FsFile taskFile;  
  if (!taskFile.open(SCHEDULE_FILE.c_str(), FILE_READ)) {
    Serial.println("open failed");
  }

  const int lenOfLine = 40;
  while (taskFile.available()) {
    char buffer[lenOfLine];                   // create buffer
    memset(buffer, 0, sizeof(buffer));        // clean the buffer 

    // save char one by one untill meet '\n' ('\n' will be ignore)
    int index = 0;
    while(1){
      char tempChar = taskFile.read();        // save to buffer 
      if(tempChar == '\n'){
        break;
      }
      buffer[index] = tempChar;
      index += 1;
    }
    buffer[index] = '\0';                     // add postfix

    // info 
    #ifdef ADD_ALL_TASK_FROM_FILE_DEBUG
      Serial.println(String(buffer) + " : " + String(index));
    #endif

    // End loop condition 
    if(String(buffer) == "#---------"){
      break;
    }

    // if not last line, add task
    myTask tempTask = parseTasks(String(buffer));
    addTask(&taskArray, &tempTask, &arrayMaxSize, &arrayUsedIndex);
  }
  
  // read finished, close file
  taskFile.close();

  // info 
  #ifdef ADD_ALL_TASK_FROM_FILE_DEBUG
    Serial.println("Done");
  #endif

  // calculate repeat
  addRepeatWorks(taskArray);

  // sort 
  #ifdef ADD_ALL_TASK_FROM_FILE_DEBUG
    Serial.println("Before sort");
    printAllTask(taskArray, arrayUsedIndex);
  #endif
  sortTask(taskArray, arrayUsedIndex);
  #ifdef ADD_ALL_TASK_FROM_FILE_DEBUG
    Serial.println("After sort");
    printAllTask(taskArray, arrayUsedIndex);
    Serial.println("arrayUsedIndex : " + String(arrayUsedIndex));
    Serial.println("");
  #endif
}


void findTheMatchedArrayReadIndex(){
  int startTimeOfNext = 0;

  while(1){
    // check 
    if( getPassedSecOfToday() < startTimeOfNext * 60 ){
      break;
    }

    // get next 
    if((taskArray + arrayReadIndex)->setType == 0){  // simple task
      startTimeOfNext = (taskArray + arrayReadIndex)->taskType.simple.start_min_of_a_day;
    }else{
      startTimeOfNext = (taskArray + arrayReadIndex)->taskType.complex.start_min_of_a_day;
    }

    // index add
    arrayReadIndex += 1;
  }

  Serial.println( "Now : " + String(secMapTo24Hour(getPassedSecOfToday())) + ", next : " + String(startTimeOfNext) + "(" + String( minConvertTohour24(startTimeOfNext) ) + ")");
}

#endif