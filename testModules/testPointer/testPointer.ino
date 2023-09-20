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


void addTask(task InputTask){
  // if array size is not enough
  if(arrayUsedIndex == arrayMaxSize){
    // create a temp array with double size
    task *tempTaskArray = (task*)malloc( sizeof(task) * arrayMaxSize * 2);
    // copy data to new array
    memcpy(tempTaskArray, taskArray, arrayMaxSize * sizeof(task) );
    // release old array ram space
    free(taskArray);
    // new array pointer give to old name
    taskArray = tempTaskArray;
    // double the variable
    arrayMaxSize = arrayMaxSize * 2;
  }

  // give value to that array slot
  *(taskArray + arrayUsedIndex) = InputTask;
  // index + 1
  arrayUsedIndex += 1;
}

void sortTask(task *taskArray, int arrayUsedIndex){
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

void printAllTask(){
  int index = 0;
  while(index != arrayUsedIndex){
    Serial.print("start_min_of_a_day : " + String( (taskArray+index)->start_min_of_a_day ));
    Serial.print(" / task : " + String( (taskArray+index)->task ));
    Serial.print(" / time : " + String( (taskArray+index)->time ));
    Serial.print(" / channel : " + String( (taskArray+index)->channel ));
    Serial.println(" / multiple : " + String( (taskArray+index)->multiple ));
    index += 1;
  }
}


// void parseTasks(){


// }

void setup() {
  Serial.begin(115200);


  for (int i=0; i<5; i++){
    task FakeTask = {
      .start_min_of_a_day = 5-arrayUsedIndex,
      .task = 'A',
      .time = 12.34-arrayUsedIndex,
      .channel = 0,
      .multiple = 22.34
    };
    addTask(FakeTask);
  }
  Serial.println("Before sort");
  printAllTask();

  sortTask(taskArray, arrayUsedIndex);
  Serial.println("After sort");
  printAllTask();
  Serial.println("");
}

void loop() {
  // put your main code here, to run repeatedly:

}
