#define PARSE_TASK_DEBUG
// #define ADD_REPEAT_WORKS_DEBUG
// #define SORT_TASK_DEBUG

int arrayMaxSize = 2;
int arrayUsedIndex = 0;

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

typedef struct task_t {
  byte setType;  // 0 for simple, 1 for complex
  union {
    simpleTask simple;
    complexTask complex;
  } taskType;
}task;

task *taskArray = (task*)malloc( sizeof(task) * arrayMaxSize );

void addTask(task **pointerToTaskArray, task *pointerToTask, int *pointerToArrayMaxSize, int *pointerToArrayUsedIndex){
  // if array size is not enough
  if(*pointerToArrayUsedIndex == *pointerToArrayMaxSize){
    // create a temp array with double size
    task *tempTaskArray = (task*)malloc( sizeof(task) * *pointerToArrayMaxSize * 2);
    // copy data to new array
    memcpy(tempTaskArray, *pointerToTaskArray, *pointerToArrayMaxSize * sizeof(task) );
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


void sortTask(task *taskArray, int arrayUsedIndex){
  // bubbleSort
	int i, j;
  task temp;
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



void printAllTask(task *taskArray, int inputArrayUsedIndex){
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

int hour24ConvetToMin(int input){
  return (input/100)*60 + input %100 ;
}

int minConvertTohour24(int input){
  return (input/60)*100 + input % 60;
}

task parseTasks(String input){
  int lenCount = 0;
  char *temp[5];  // a pointer point to a value which can storage 5 char array pointer

  char *token;
  temp[lenCount] = strtok(input.c_str(), ",");
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

  task tempTask;
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
    tempTask.taskType.complex.channel = atoi(temp[3][0]);
    tempTask.taskType.complex.multiple = atof(temp[4]);
  }
  return tempTask;
}


void addRepeatWorks(task *inputTaskArray){
  // new variable and larger size array
  int tempArrayMaxSize = 2;
  int tempArrayUsedIndex = 0;
  task *temptaskArray = (task*)malloc( sizeof(task) * tempArrayMaxSize );

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
        task tempTask;        // !!same address, if malloc will be different!!
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


void setup() {
  Serial.begin(115200);

  String testInput[] = {
    /*
    // 0.083hr, 5 min, total 288
    // 0.166hr, 10min, total 144
    // 0.25hr, 15 min, total 96
    // 3hr, total 8 
    // 6hr, total 4
    // 12hr, total 2 
    // 24hr, total 1
    */

    "1345,A,10,B,1",
    "0000,A,30,L,1",
    "1900,A,20,B,0.5",
    "0325,A,15,R,2",
    "0000,B,0.166",  
    "0000,C,0.25",   
    "0000,D,3"       
  };


  for(int i=0; i<7; i++){
    addTask(&taskArray, &parseTasks(testInput[i]), &arrayMaxSize, &arrayUsedIndex);
  }
  addRepeatWorks(taskArray);

  Serial.println("Before sort");
  printAllTask(taskArray, arrayUsedIndex);
  sortTask(taskArray, arrayUsedIndex);
  Serial.println("After sort");
  printAllTask(taskArray, arrayUsedIndex);
  Serial.println("arrayUsedIndex : " + String(arrayUsedIndex));
  Serial.println("");
}

void loop() {
  // put your main code here, to run repeatedly:

}
