# Monitored Item 추가

- 헤더 파일 추가
```c
#include <open62541/client_subscriptions.h>
```
- Notification 함수 정의
```c
static void
dataChangeNotificationCallback(UA_Server* server, UA_UInt32 monitoredItemId,
    void* monitoredItemContext, const UA_NodeId* nodeId,
    void* nodeContext, UA_UInt32 attributeId,
    const UA_DataValue* value) {
    UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND,
        "Received Notification");
}
```
- Monitored Item 구현 함수 정의
```c
static void
addMonitoredItem(UA_Server* server) {
    UA_NodeId MonitoredItemNodeId = UA_NODEID_NUMERIC(2, 6010);
    UA_MonitoredItemCreateRequest monRequest =
        UA_MonitoredItemCreateRequest_default(MonitoredItemNodeId);
    monRequest.requestedParameters.samplingInterval = 3000.0; /*
    1 s interval */
    UA_Server_createDataChangeMonitoredItem(server, UA_TIMESTAMPSTORETURN_SOURCE,
        monRequest, NULL, dataChangeNotificationCallback);
}
```
- Main 함수에서 함수 호출
```c
addMonitoredItem(server);
```
