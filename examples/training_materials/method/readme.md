# Method 추가

- 전역변수 추가
```c
UA_Boolean tempBool = true;
UA_UInt64 callbackId;
```

- onOffMethodCallback 함수 정의
    - afterCallbackAngleNode 코드 아래에 입력
```c
onOffMethodCallback(UA_Server* server,
    const UA_NodeId* sessionId, void* sessionHandle,
    const UA_NodeId* methodId, void* methodContext,
    const UA_NodeId* objectId, void* objectContext,
    size_t inputSize, const UA_Variant* input,
    size_t outputSize, UA_Variant* output) {
    UA_NodeId AngleNode = UA_NODEID_NUMERIC(2, 6010);
    UA_NodeId StatusNode = UA_NODEID_NUMERIC(2, 6007);


    UA_Variant value;
    UA_StatusCode retval;

    UA_ValueCallback callback;
    callback.onRead = NULL;
    callback.onWrite = afterWriteCallbackAngleNode;


    if (tempBool == true) {

        retval = UA_Server_setVariableNode_valueCallback(server, AngleNode, callback);
        if (retval != UA_STATUSCODE_GOOD) {
            UA_LOG_ERROR(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND,
                "setting AngleNode Callback failed. StatusCode %s", UA_StatusCode_name(retval));
            return retval;
        }
        UA_Boolean robotStatus = true;
        UA_Variant_setScalar(&value, &robotStatus, &UA_TYPES[UA_TYPES_BOOLEAN]);
        retval = UA_Server_writeValue(server, StatusNode, value);
        if (retval != UA_STATUSCODE_GOOD) {
            UA_LOG_ERROR(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND,
                "Changing status node failed. StatusCode %s", UA_StatusCode_name(retval));
            return retval;
        }
        UA_Server_addRepeatedCallback(server, writeRandomVariable, NULL, 10000, &callbackId);  // 10초에 한 번씩 writeRandomVariable 함수 호출
        UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_SERVER, "Robot is turned on");

        tempBool = false;
    }
    else {
        UA_Server_removeCallback(server, callbackId);

        UA_Boolean robotStatus = false;
        UA_Variant_setScalar(&value, &robotStatus, &UA_TYPES[UA_TYPES_BOOLEAN]);
        retval = UA_Server_writeValue(server, StatusNode, value);
        if (retval != UA_STATUSCODE_GOOD) {
            UA_LOG_ERROR(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND,
                "Changing status node failed. StatusCode %s", UA_StatusCode_name(retval));
            return retval;
        }

        UA_Int32 robotAnlge = 0;
        UA_Variant_setScalar(&value, &robotAnlge, &UA_TYPES[UA_TYPES_INT32]);
        retval = UA_Server_writeValue(server, AngleNode, value);
        if (retval != UA_STATUSCODE_GOOD) {
            UA_LOG_ERROR(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND,
                "Changing status node failed. StatusCode %s", UA_StatusCode_name(retval));
            return retval;
        }
        UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_SERVER, "Robot is turned off");

        tempBool = true;
    }

    return UA_STATUSCODE_GOOD;
}
```

- addRobotStatusMethod 함수 정의
    - 계속해서 이어서 입력
```c
static void
addRobotStatusMethod(UA_Server* server) {

    UA_MethodAttributes onOffAttr = UA_MethodAttributes_default;
    onOffAttr.description = UA_LOCALIZEDTEXT("en-US", "On/Off status");
    onOffAttr.displayName = UA_LOCALIZEDTEXT("en-US", "On/Off");
    onOffAttr.executable = true;
    onOffAttr.userExecutable = true;
    UA_Server_addMethodNode(server, UA_NODEID_NUMERIC(1, 10000),
        conditionSource,
        UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT),
        UA_QUALIFIEDNAME(1, "On/Off"),
        onOffAttr, &onOffMethodCallback,
        0, NULL, 0, NULL, NULL, NULL);
}
```
- SetUpEnvironment 함수 수정
    - Angle Node 변수 제거
    - addRobotStatusMethod 함수 호출
    - afterWriteCallbackAngleNode 호출 코드 제거
```c
//UA_NodeId AngleNode = UA_NODEID_NUMERIC(2, 6010);
```
```c
addRobotStatusMethod(server);
```
```c
/*
callback.onWrite = afterWriteCallbackAngleNode;
    retval = UA_Server_setVariableNode_valueCallback(server, AngleNode, callback);
    if (retval != UA_STATUSCODE_GOOD) {
        UA_LOG_ERROR(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND,
            "setting AngleNode Callback failed. StatusCode %s", UA_StatusCode_name(retval));
        return retval;
    }
*/
```
- ConditionSourceObject와 Robot1 Instance 간 Reference 생성
    - addConditionSourceObject 맨 아래 return 위에 추가
```c
retval = UA_Server_addReference(server, UA_NODEID_NUMERIC(2, 5004),//Robot1 Instance
        UA_NODEID_NUMERIC(0, UA_NS0ID_HASNOTIFIER),
        UA_EXPANDEDNODEID_NUMERIC(conditionSource.namespaceIndex,
            conditionSource.identifier.numeric),
        UA_TRUE);
```
- Main 함수 설정
    - UA_Server_addRepeatedCallback 코드 제거
```c
//UA_Server_addRepeatedCallback(server, writeRandomVariable, NULL, 5000, NULL);
```
- Callback 제거 함수 설정
    - afterWriteCallbackAngle 함수 내부에 있는 angle 값 300 초과 조건에 입력
    - onOffMethodCallback 함수 내부에 있는 else 조건에 입력
```c
UA_Server_removeCallback(server, callbackId);
```