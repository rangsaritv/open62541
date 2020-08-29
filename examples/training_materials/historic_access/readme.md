# Historic Access 추가

- 헤더파일 추가
```c
//historic library
#include <open62541/plugin/historydata/history_data_backend_memory.h>
#include <open62541/plugin/historydata/history_data_gathering_default.h>
#include <open62541/plugin/historydata/history_database_default.h>
#include <open62541/plugin/historydatabase.h>
```

- 서버 설정 부분 변경
```c
//UA_ServerConfig_setDefault(UA_Server_getConfig(server));
UA_ServerConfig* config = UA_Server_getConfig(server);
UA_ServerConfig_setDefault(config);
```

- Hisotic access 설정
    - Main 함수 if(namespace_robot_model_generated(server) != UA_STATUSCODE_GOOD) 코드 위에 추가
```c
UA_HistoryDataGathering gathering = UA_HistoryDataGathering_Default(1);
config->historyDatabase = UA_HistoryDatabase_default(gathering);
UA_NodeId historicNodeId = UA_NODEID_NUMERIC(2, 6010);
```
- Historic Node 설정
    - Main 함수 else 구문 시작 부분에 추가
```c
/************* Historic Access ***************/
UA_HistorizingNodeIdSettings setting;

setting.historizingBackend = UA_HistoryDataBackend_Memory(3, 100);
setting.maxHistoryDataResponseSize = 100;
setting.historizingUpdateStrategy = UA_HISTORIZINGUPDATESTRATEGY_VALUESET;

retval = gathering.registerNodeId(server, gathering.context, &historicNodeId, setting);
UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_SERVER, "registerNodeId %s", UA_StatusCode_name(retval));
```