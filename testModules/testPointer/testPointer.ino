// #define PARSE_TASK_DEBUG


int arrayMaxSize = 2;
int arrayUsedIndex = 0;

typedef struct task_t{
  int start_min_of_a_day;
  char task;
  float time;
  int channel;
  float multiple;
}task;

task *taskArray = (task*)malloc( sizeof(task) * arrayMaxSize );

void addTask(task **pointerToTaskArray, task *pointerToTask, int *pointerToArrayMaxSize, int *pointerToArrayUsedIndex){
  Serial.println("pointerToTask : " + String(int(pointerToTask)));

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

  // give value to that array slot
  // Serial.println("*pointerToArrayUsedIndex : " + String(*pointerToArrayUsedIndex));
  *(*pointerToTaskArray + *pointerToArrayUsedIndex) = *pointerToTask;
  // memcpy(*pointerToTaskArray + *pointerToArrayUsedIndex, pointerToTask, sizeof(task) );
  // index + 1
  *pointerToArrayUsedIndex += 1;
}

void sortTask(task *taskArray, int arrayUsedIndex){
  // quick sort
  for (int i = 0; i < arrayUsedIndex; i++) {
    int j = i;
    while (j > 0 && taskArray[j - 1].start_min_of_a_day > taskArray[j].start_min_of_a_day) {
      task temp = taskArray[j];
      taskArray[j] = taskArray[j - 1];
      taskArray[j - 1] = temp;
      j--;
    }
  }
}

void printAllTask(task *taskArray, int inputArrayUsedIndex){
  for(int index = 0; index < inputArrayUsedIndex; index++){
    Serial.println("Address : " + String( (int)(taskArray+index)) );
    Serial.print("start_min_of_a_day : " + String( (taskArray+index)->start_min_of_a_day ));
    Serial.print(" / task : " + String( (taskArray+index)->task ));
    Serial.print(" / time : " + String( (taskArray+index)->time ));
    Serial.print(" / channel : " + String( (taskArray+index)->channel ));
    Serial.println(" / multiple : " + String( (taskArray+index)->multiple ));
  }
}

int hour24ConvetToMin(int input){
  return (input/100)*60 + input %100 ;
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
  if(lenCount == 3){
    tempTask.start_min_of_a_day = hour24ConvetToMin(atoi(temp[0]));
    tempTask.task = temp[1][0];
    tempTask.time = atof(temp[2]);
    tempTask.channel = ' ';
    tempTask.multiple = 0.0;
  }else{
    tempTask.start_min_of_a_day = hour24ConvetToMin(atoi(temp[0]));
    tempTask.task = temp[1][0];
    tempTask.time = atof(temp[2]);
    tempTask.channel = atoi(temp[3][0]);
    tempTask.multiple = atof(temp[4]);
  }
  return tempTask;
}


void addRepeatWorks(task **taskArray){
  int tempArrayMaxSize = 2;
  int tempArrayUsedIndex = 0;
  task *temptaskArray = (task*)malloc( sizeof(task) * tempArrayMaxSize );

  Serial.println("arrayUsedIndex" + String(arrayUsedIndex));
  for (int i=0; i<arrayUsedIndex; i++){
    if((*taskArray+i)->task == 'A'){
      Serial.println("direct copy" + String(i));
      addTask(&temptaskArray, (*taskArray+i), &tempArrayMaxSize, &tempArrayUsedIndex);
      Serial.println("tempArrayMaxSize : " + String(tempArrayMaxSize));
      Serial.println("tempArrayUsedIndex : " + String(tempArrayUsedIndex));
    }else{
      // count repeat time
      int repeatTime = 24.0 / ((*taskArray+i)->time);
      // Serial.println("repeatTime : " + String(repeatTime));

      for (int j=0; j<repeatTime; j++){
        Serial.println("loop create: " + String(j+1));
        task tempTask;
        Serial.println("Address : " + String((int)(&tempTask)));  // same address, malloc different

        tempTask.time = 0.0;
        tempTask.channel = 0;
        tempTask.multiple = 0.0;
        tempTask.task = ( (*taskArray+i)->task );
        tempTask.start_min_of_a_day = ( (*taskArray+i)->start_min_of_a_day) + j * (int)((*taskArray+i)->time) * 60;

        // Serial.println("(taskArray+i)->task" + String((taskArray+i)->task));
        // Serial.println("(taskArray+i)->start_min_of_a_day)" + String((taskArray+i)->start_min_of_a_day) );

        // int s_time = ((taskArray+i)->start_min_of_a_day);
        // float cutt_time = ((taskArray+i)->time);
        // Serial.println( s_time + j * cutt_time * 60 );

        
        addTask(&temptaskArray, &tempTask, &tempArrayMaxSize, &tempArrayUsedIndex);
        Serial.println("tempArrayMaxSize : " + String(tempArrayMaxSize));
        Serial.println("tempArrayUsedIndex : " + String(tempArrayUsedIndex));
      }
    }
  }

  printAllTask(temptaskArray, tempArrayUsedIndex);


  // replace with new array
  arrayMaxSize = tempArrayMaxSize;
  arrayUsedIndex = tempArrayUsedIndex;
  taskArray = &temptaskArray;
}


void setup() {
  Serial.begin(115200);


  // for (int i=0; i<5; i++){
  //   task FakeTask = {
  //     .start_min_of_a_day = 5-arrayUsedIndex,
  //     .task = 'A',
  //     .time = 12.34-arrayUsedIndex,
  //     .channel = 0,
  //     .multiple = 22.34
  //   };
  //   addTask(FakeTask);
  // }
  // Serial.println("Before sort");
  // printAllTask(taskArray, arrayUsedIndex);

  // sortTask(taskArray, arrayUsedIndex);
  // Serial.println("After sort");
  // printAllTask(taskArray, arrayUsedIndex);
  // Serial.println("");


  String testInput[] = {
    "0000,A,30,L,1",
    "0325,A,15,R,2",
    "1345,A,10,B,1",
    "1900,A,20,B,0.5",
    "0000,B,12",
    "0010,C,24",
    "0020,D,24"
  };


  for(int i=0; i<7; i++){
    addTask(&taskArray, &parseTasks(testInput[i]), &arrayMaxSize, &arrayUsedIndex);
  }
  addRepeatWorks(&taskArray);

  // Serial.println("Before sort");
  // printAllTask(taskArray, arrayUsedIndex);
  // sortTask(taskArray, arrayUsedIndex);
  // Serial.println("After sort");
  // printAllTask(taskArray, arrayUsedIndex);
  Serial.println("");
}

void loop() {
  // put your main code here, to run repeatedly:

}
