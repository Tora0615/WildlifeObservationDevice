#ifndef TASKFILEREADER_H
#define TASKFILEREADER_H


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

#define CSV_DELIM ','

int csvReadText(File* file, char* str, size_t size, char delim) {
  char ch;
  int rtn;
  size_t n = 0;
  while (true) {
    // check for EOF
    if (!file->available()) {
      rtn = 0;
      break;
    }
    if (file->read(&ch, 1) != 1) {
      // read error
      rtn = -1;
      break;
    }
    // Delete CR.
    if (ch == '\r') {
      continue;
    }
    if (ch == delim || ch == '\n') {
      rtn = ch;
      break;
    }
    if ((n + 1) >= size) {
      // string too long
      rtn = -2;
      n--;
      break;
    }
    str[n++] = ch;
  }
  str[n] = '\0';
  return rtn;
}
//------------------------------------------------------------------------------
int csvReadInt32(File* file, int32_t* num, char delim) {
  char buf[20];
  char* ptr;
  int rtn = csvReadText(file, buf, sizeof(buf), delim);
  if (rtn < 0) return rtn;
  *num = strtol(buf, &ptr, 10);
  if (buf == ptr) return -3;
  while(isspace(*ptr)) ptr++;
  return *ptr == 0 ? rtn : -4;
}
//------------------------------------------------------------------------------
int csvReadInt16(File* file, int16_t* num, char delim) {
  int32_t tmp;
  int rtn = csvReadInt32(file, &tmp, delim);
  if (rtn < 0) return rtn;
  if (tmp < INT_MIN || tmp > INT_MAX) return -5;
  *num = tmp;
  return rtn;
}
//------------------------------------------------------------------------------
int csvReadUint32(File* file, uint32_t* num, char delim) {
  char buf[20];
  char* ptr;
  int rtn = csvReadText(file, buf, sizeof(buf), delim);
  if (rtn < 0) return rtn;
  *num = strtoul(buf, &ptr, 10);
  if (buf == ptr) return -3;
  while(isspace(*ptr)) ptr++;
  return *ptr == 0 ? rtn : -4;
}
//------------------------------------------------------------------------------
int csvReadUint16(File* file, uint16_t* num, char delim) {
  uint32_t tmp;
  int rtn = csvReadUint32(file, &tmp, delim);
  if (rtn < 0) return rtn;
  if (tmp > UINT_MAX) return -5;
  *num = tmp;
  return rtn;
}
//------------------------------------------------------------------------------
int csvReadDouble(File* file, double* num, char delim) {
  char buf[20];
  char* ptr;
  int rtn = csvReadText(file, buf, sizeof(buf), delim);
  if (rtn < 0) return rtn;
  *num = strtod(buf, &ptr);
  if (buf == ptr) return -3;
  while(isspace(*ptr)) ptr++;
  return *ptr == 0 ? rtn : -4;
}
//------------------------------------------------------------------------------
int csvReadFloat(File* file, float* num, char delim) {
  double tmp;
  int rtn = csvReadDouble(file, &tmp, delim);
  if (rtn < 0)return rtn;
  // could test for too large.
  *num = tmp;
  return rtn;
}
//------------------------------------------------------------------------------





#ENDIF