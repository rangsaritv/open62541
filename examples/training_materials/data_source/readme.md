# 랜덤 함수 추가

- 랜덤 값 생성 함수 정의
```c
static void
writeRandomVariable(UA_Server* server, void* data) {
    UA_NodeId myIntegerNodeId = UA_NODEID_NUMERIC(2, 6010);
    /* Write a different integer value */
    UA_Int32 myInteger = UA_UInt32_random() % 360;
    UA_Variant myVar;
    UA_Variant_init(&myVar);
    UA_Variant_setScalar(&myVar, &myInteger, &UA_TYPES[UA_TYPES_INT32]);
    UA_Server_writeValue(server, myIntegerNodeId, myVar);
}
```
- Main 함수에 적용
	- UA_Server_addRepeatedCallback 함수를 이용해서 호출

```c
UA_Server_addRepeatedCallback(server, writeRandomVariable, NULL, 5000, NULL);
```
