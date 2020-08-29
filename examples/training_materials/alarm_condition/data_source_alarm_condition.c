/* This work is licensed under a Creative Commons CCZero 1.0 Universal License.
 * See http://creativecommons.org/publicdomain/zero/1.0/ for more information. */

#include <open62541/plugin/log_stdout.h>
#include <open62541/server.h>
#include <open62541/server_config_default.h>
#include <open62541/client_subscriptions.h>

 //historic library
#include <open62541/plugin/historydata/history_data_backend_memory.h>
#include <open62541/plugin/historydata/history_data_gathering_default.h>
#include <open62541/plugin/historydata/history_database_default.h>
#include <open62541/plugin/historydatabase.h>

#include "open62541/namespace_robot_model_generated.h"

#include <signal.h>
#include <stdlib.h>

UA_Boolean running = true;
static UA_NodeId conditionSource;
static UA_NodeId angleConditionInstance;

static void stopHandler(int sign) {
    UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_SERVER, "received ctrl-c");
    running = false;
}

static void
writeRandomVariable(UA_Server* server, void* data) {
    UA_NodeId myIntegerNodeId = UA_NODEID_NUMERIC(2, 6008);

    UA_Int32 myInteger = UA_UInt32_random() % 360;  // 0 ~ 359 사이 랜덤 값 생성
    UA_Variant myVar;
    UA_Variant_init(&myVar);
    UA_Variant_setScalar(&myVar, &myInteger, &UA_TYPES[UA_TYPES_INT32]);
    UA_Server_writeValue(server, myIntegerNodeId, myVar);

}

static void
dataChangeNotificationCallback(UA_Server* server, UA_UInt32 monitoredItemId,
    void* monitoredItemContext, const UA_NodeId* nodeId,
    void* nodeContext, UA_UInt32 attributeId,
    const UA_DataValue* value) {
    UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND,
        "Received Notification");
}

static void
addMonitoredItem(UA_Server* server) {
    UA_NodeId MonitoredItemNodeId = UA_NODEID_NUMERIC(2, 6008);
    UA_MonitoredItemCreateRequest monRequest =
        UA_MonitoredItemCreateRequest_default(MonitoredItemNodeId);
    monRequest.requestedParameters.samplingInterval = 10000;
    UA_Server_createDataChangeMonitoredItem(server, UA_TIMESTAMPSTORETURN_SOURCE,
        monRequest, NULL, dataChangeNotificationCallback);
}

static UA_StatusCode
addConditionSourceObject(UA_Server* server) {
    UA_ObjectAttributes object_attr = UA_ObjectAttributes_default;
    object_attr.eventNotifier = 1;  // SubscribeToEvents로 설정해서 event 발생하는 것을 subscribe한다는 의미

    object_attr.displayName = UA_LOCALIZEDTEXT("en", "ConditionSourceObject");
    UA_StatusCode retval = UA_Server_addObjectNode(server, UA_NODEID_NULL,
        UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER),
        UA_NODEID_NUMERIC(0, UA_NS0ID_ORGANIZES),
        UA_QUALIFIEDNAME(0, "ConditionSourceObject"),
        UA_NODEID_NUMERIC(0, UA_NS0ID_BASEOBJECTTYPE),
        object_attr, NULL, &conditionSource);

    retval = UA_Server_addReference(server, UA_NODEID_NUMERIC(2, 5004), //robot1 instance
        UA_NODEID_NUMERIC(0, UA_NS0ID_HASNOTIFIER),
        UA_EXPANDEDNODEID_NUMERIC(conditionSource.namespaceIndex,
            conditionSource.identifier.numeric),
        UA_TRUE);

    return retval;
}

static UA_StatusCode
addAngleCondition(UA_Server* server) {  // Condition 생성
    UA_StatusCode retval = addConditionSourceObject(server);    // Condition source object 생성
    if (retval != UA_STATUSCODE_GOOD) {
        UA_LOG_ERROR(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND,
            "creating Condition Source failed. StatusCode %s", UA_StatusCode_name(retval)); // 생성 오류시 로그 출력
    }
    retval = UA_Server_createCondition(server,  // Condition source object 하위에 Condition instance 생성
        UA_NODEID_NULL, UA_NODEID_NUMERIC(0, UA_NS0ID_OFFNORMALALARMTYPE),
        UA_QUALIFIEDNAME(0, "angleCondition"), conditionSource,
        UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT), &angleConditionInstance);

    return retval;
}
static UA_StatusCode
enteringEnabledStateCallback(UA_Server* server, const UA_NodeId* condition) {
    UA_Boolean retain = true;
    return UA_Server_writeObjectProperty_scalar(server, *condition,
        UA_QUALIFIEDNAME(0, "Retain"),
        &retain,
        &UA_TYPES[UA_TYPES_BOOLEAN]);
}
static UA_StatusCode
enteringAckedStateCallback(UA_Server* server, const UA_NodeId* condition) {
    /* deactivate Alarm when acknowledging*/
    UA_Boolean activeStateId = false;
    UA_Variant value;
    UA_QualifiedName activeStateField = UA_QUALIFIEDNAME(0, "ActiveState");
    UA_QualifiedName activeStateIdField = UA_QUALIFIEDNAME(0, "Id");

    UA_Variant_setScalar(&value, &activeStateId, &UA_TYPES[UA_TYPES_BOOLEAN]);
    UA_StatusCode retval =
        UA_Server_setConditionVariableFieldProperty(server, *condition,
            &value, activeStateField,
            activeStateIdField);

    if (retval != UA_STATUSCODE_GOOD) {
        UA_LOG_ERROR(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND,
            "Setting ActiveState/Id Field failed. StatusCode %s",
            UA_StatusCode_name(retval));
    }

    return retval;
}
static UA_StatusCode
enteringConfirmedStateCallback(UA_Server* server, const UA_NodeId* condition) {
    /* Deactivate Alarm and put it out of the interesting state (by writing
     * false to Retain field) when confirming*/
    UA_Boolean activeStateId = false;
    UA_Boolean retain = false;
    UA_Variant value;
    UA_QualifiedName activeStateField = UA_QUALIFIEDNAME(0, "ActiveState");
    UA_QualifiedName activeStateIdField = UA_QUALIFIEDNAME(0, "Id");
    UA_QualifiedName retainField = UA_QUALIFIEDNAME(0, "Retain");

    UA_Variant_setScalar(&value, &activeStateId, &UA_TYPES[UA_TYPES_BOOLEAN]);
    UA_StatusCode retval =
        UA_Server_setConditionVariableFieldProperty(server, *condition,
            &value, activeStateField,
            activeStateIdField);
    if (retval != UA_STATUSCODE_GOOD) {
        UA_LOG_ERROR(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND,
            "Setting ActiveState/Id Field failed. StatusCode %s",
            UA_StatusCode_name(retval));
        return retval;
    }

    UA_Variant_setScalar(&value, &retain, &UA_TYPES[UA_TYPES_BOOLEAN]);
    retval = UA_Server_setConditionField(server, *condition,
        &value, retainField);
    if (retval != UA_STATUSCODE_GOOD) {
        UA_LOG_ERROR(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND,
            "Setting ActiveState/Id Field failed. StatusCode %s",
            UA_StatusCode_name(retval));
    }

    return retval;
}
static void
afterWriteCallbackAngleNode(UA_Server* server,
    const UA_NodeId* sessionId, void* sessionContext,
    const UA_NodeId* nodeId, void* nodeContext,
    const UA_NumericRange* range, const UA_DataValue* data) {


    UA_QualifiedName activeStateField = UA_QUALIFIEDNAME(0, "ActiveState");
    UA_QualifiedName messageField = UA_QUALIFIEDNAME(0, "Message");
    UA_QualifiedName idField = UA_QUALIFIEDNAME(0, "Id");

    UA_NodeId AngleNode = UA_NODEID_NUMERIC(2, 6008);
    UA_NodeId StatusNode = UA_NODEID_NUMERIC(2, 6009);

    UA_StatusCode retval = UA_Server_writeObjectProperty_scalar(server, angleConditionInstance, UA_QUALIFIEDNAME(0, "Time"),
        &data->serverTimestamp, &UA_TYPES[UA_TYPES_DATETIME]);
    UA_Variant value;
    UA_Boolean idValue = false;

    if (*(UA_Int32*)(data->value.data) > 300) {


        UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_SERVER, "over 300 %d", *(UA_Int32*)(data->value.data));
        UA_Boolean robotStatus = false;
        UA_Variant_setScalar(&value, &robotStatus, &UA_TYPES[UA_TYPES_BOOLEAN]);
        retval = UA_Server_writeValue(server, StatusNode, value);
        if (retval != UA_STATUSCODE_GOOD) {
            UA_LOG_ERROR(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND,
                "Changing status node failed. StatusCode %s", UA_StatusCode_name(retval));
            return;
        }
        UA_Int32 robotAngle = 0;
        UA_Variant_setScalar(&value, &robotAngle, &UA_TYPES[UA_TYPES_INT32]);
        retval = UA_Server_writeValue(server, AngleNode, value);
        if (retval != UA_STATUSCODE_GOOD) {
            UA_LOG_ERROR(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND,
                "Changing Angle node failed. StatusCode %s", UA_StatusCode_name(retval));
            return;
        }
        UA_LocalizedText messageValue = UA_LOCALIZEDTEXT("en", "Danger & Stop");
        UA_Variant_setScalar(&value, &messageValue, &UA_TYPES[UA_TYPES_LOCALIZEDTEXT]);
        retval = UA_Server_setConditionField(server, angleConditionInstance,
            &value, messageField);
        if (retval != UA_STATUSCODE_GOOD) {
            UA_LOG_ERROR(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND,
                "Setting Message Field failed. StatusCode %s", UA_StatusCode_name(retval));
            return;
        }

        retval = UA_Server_triggerConditionEvent(server, angleConditionInstance, conditionSource, NULL);
        if (retval != UA_STATUSCODE_GOOD) {
            UA_LOG_WARNING(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND,
                "Triggering condition event failed. StatusCode %s", UA_StatusCode_name(retval));
            return;
        }
    }

    else if (*(UA_Int32*)(data->value.data) > 200) {

        UA_Boolean activeStateId = true;
        UA_Variant_setScalar(&value, &activeStateId, &UA_TYPES[UA_TYPES_BOOLEAN]);
        retval |= UA_Server_setConditionVariableFieldProperty(server, angleConditionInstance,
            &value, activeStateField,
            idField);
        if (retval != UA_STATUSCODE_GOOD) {
            UA_LOG_ERROR(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND,
                "Setting ActiveState/Id Field failed. StatusCode %s",
                UA_StatusCode_name(retval));
            return;
        }

        UA_LocalizedText messageValue = UA_LOCALIZEDTEXT("en", "Warning");
        UA_Variant_setScalar(&value, &messageValue, &UA_TYPES[UA_TYPES_LOCALIZEDTEXT]);
        retval = UA_Server_setConditionField(server, angleConditionInstance,
            &value, messageField);
        if (retval != UA_STATUSCODE_GOOD) {
            UA_LOG_ERROR(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND,
                "Setting Message Field failed. StatusCode %s", UA_StatusCode_name(retval));
            return;
        }

        retval = UA_Server_triggerConditionEvent(server, angleConditionInstance, conditionSource, NULL);
        if (retval != UA_STATUSCODE_GOOD) {
            UA_LOG_WARNING(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND,
                "Triggering condition event failed. StatusCode %s", UA_StatusCode_name(retval));
            return;
        }
    }
    else {
        UA_Boolean activeStateId = false;
        UA_Variant_setScalar(&value, &activeStateId, &UA_TYPES[UA_TYPES_BOOLEAN]);
        retval = UA_Server_setConditionVariableFieldProperty(server, angleConditionInstance,
            &value, activeStateField,
            idField);
        if (retval != UA_STATUSCODE_GOOD) {
            UA_LOG_ERROR(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND,
                "Setting ActiveState/Id Field failed. StatusCode %s",
                UA_StatusCode_name(retval));
            return;
        }
    }
}

static UA_StatusCode
setUpEnvironment(UA_Server* server) {
    UA_NodeId AngleNode = UA_NODEID_NUMERIC(2, 6008);
    UA_ValueCallback callback;
    callback.onRead = NULL;

    UA_StatusCode retval = addAngleCondition(server);
    if (retval != UA_STATUSCODE_GOOD) {
        UA_LOG_ERROR(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND,
            "adding condition failed. StatusCode %s", UA_StatusCode_name(retval));
        return retval;
    }

    UA_TwoStateVariableChangeCallback userSpecificCallback = enteringEnabledStateCallback;
    retval = UA_Server_setConditionTwoStateVariableCallback(server, angleConditionInstance,
        conditionSource, false,
        userSpecificCallback,
        UA_ENTERING_ENABLEDSTATE);

    if (retval != UA_STATUSCODE_GOOD) {
        UA_LOG_ERROR(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND,
            "adding entering enabled state callback failed. StatusCode %s",
            UA_StatusCode_name(retval));
        return retval;
    }
    userSpecificCallback = enteringAckedStateCallback;
    retval = UA_Server_setConditionTwoStateVariableCallback(server, angleConditionInstance,
        conditionSource, false,
        userSpecificCallback,
        UA_ENTERING_ACKEDSTATE);
    if (retval != UA_STATUSCODE_GOOD) {
        UA_LOG_ERROR(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND,
            "adding entering acked state callback failed. StatusCode %s",
            UA_StatusCode_name(retval));
        return retval;
    }

    userSpecificCallback = enteringConfirmedStateCallback;
    retval = UA_Server_setConditionTwoStateVariableCallback(server, angleConditionInstance,
        conditionSource, false,
        userSpecificCallback,
        UA_ENTERING_CONFIRMEDSTATE);
    if (retval != UA_STATUSCODE_GOOD) {
        UA_LOG_ERROR(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND,
            "adding entering confirmed state callback failed. StatusCode %s",
            UA_StatusCode_name(retval));
        return retval;
    }

    callback.onWrite = afterWriteCallbackAngleNode;
    retval = UA_Server_setVariableNode_valueCallback(server, AngleNode, callback);
    if (retval != UA_STATUSCODE_GOOD) {
        UA_LOG_ERROR(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND,
            "setting AngleNode Callback failed. StatusCode %s", UA_StatusCode_name(retval));
        return retval;
    }
    return retval;
}

int main(int argc, char** argv) {
    signal(SIGINT, stopHandler);
    signal(SIGTERM, stopHandler);

    UA_Server* server = UA_Server_new();
    UA_ServerConfig* config = UA_Server_getConfig(server);
    UA_ServerConfig_setDefault(config);

    UA_StatusCode retval;

    UA_HistoryDataGathering gathering = UA_HistoryDataGathering_Default(1); //data를 수집하는 변수
    config->historyDatabase = UA_HistoryDatabase_default(gathering);
    UA_NodeId historicNodeId = UA_NODEID_NUMERIC(2, 6008);

    /* create nodes from nodeset */
    if (namespace_robot_model_generated(server) != UA_STATUSCODE_GOOD) {
        UA_LOG_ERROR(UA_Log_Stdout, UA_LOGCATEGORY_SERVER, "Could not add the example nodeset. "
            "Check previous output for any error.");
        retval = UA_STATUSCODE_BADUNEXPECTEDERROR;
    }
    else {
        /*****************Historic Access*****************/
        UA_HistorizingNodeIdSettings setting;

        setting.historizingBackend = UA_HistoryDataBackend_Memory(3, 100);  //3개 Node 100개 저장할 수 있는 저장공간
        setting.maxHistoryDataResponseSize = 100;   //한 번에 100개 데이터까지 응답 가능
        setting.historizingUpdateStrategy = UA_HISTORIZINGUPDATESTRATEGY_VALUESET;  //Update 어떻게 할지

        retval = gathering.registerNodeId(server, gathering.context, &historicNodeId, setting); //setting 설정
        UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_SERVER, "registerNodeId %s", UA_StatusCode_name(retval));

        UA_Server_addRepeatedCallback(server, writeRandomVariable, NULL, 20000, NULL);
        addMonitoredItem(server);
        setUpEnvironment(server);

        retval = UA_Server_run(server, &running);
    }

    UA_Server_delete(server);
    return retval == UA_STATUSCODE_GOOD ? EXIT_SUCCESS : EXIT_FAILURE;
}
