# Alarm & Condition 추가

- 전역변수 추가
```c
static UA_NodeId conditionSource;
static UA_NodeId angleConditionInstance;
```

- AddConditionSourceObject 함수 정의
    - addMonitoredItem 함수 아래에 입력
```c
static UA_StatusCode
addConditionSourceObject(UA_Server* server) {
    UA_ObjectAttributes object_attr = UA_ObjectAttributes_default;
    object_attr.eventNotifier = 1;

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
```

- addAngleCondition 함수 정의
    - 계속해서 이어서 입력
```c
static UA_StatusCode
addAngleCondition(UA_Server* server) {  
    UA_StatusCode retval = addConditionSourceObject(server);  
    if (retval != UA_STATUSCODE_GOOD) {
        UA_LOG_ERROR(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND,
            "creating Condition Source failed. StatusCode %s", UA_StatusCode_name(retval)); 
    }
    retval = UA_Server_createCondition(server,  
        UA_NODEID_NULL, UA_NODEID_NUMERIC(0, UA_NS0ID_OFFNORMALALARMTYPE),
        UA_QUALIFIEDNAME(0, "angleCondition"), conditionSource,
        UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT), &angleConditionInstance);

    return retval;
}
```
- enteringEnabledStateCallback 함수 정의
    - 계속해서 이어서 입력
```c
static UA_StatusCode
enteringEnabledStateCallback(UA_Server* server, const UA_NodeId* condition) {
    UA_Boolean retain = true;
    return UA_Server_writeObjectProperty_scalar(server, *condition,
        UA_QUALIFIEDNAME(0, "Retain"),
        &retain,
        &UA_TYPES[UA_TYPES_BOOLEAN]);
}
```
- enteringAckedStateCallback 함수 정의
    - 계속해서 이어서 입력
```c
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
```
- enteringConfirmedStateCallback 함수 정의
    - 계속해서 이어서 입력
```c
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
```

- afterWriteCallbackAngleNode 함수 정의
    - 계속해서 이어서 입력
```c
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

        retval = UA_Server_triggerConditionEvent(server, angleConditionInstance,
            conditionSource, NULL);
        if (retval != UA_STATUSCODE_GOOD) {
            UA_LOG_WARNING(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND,
                "Triggering condition event failed. StatusCode %s",
                UA_StatusCode_name(retval));
            return;
        }
    }
}
```

- setUpEnvironment 함수 정의
    - 계속해서 이어서 입력
```c
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

```

- Main 함수 설정
    - AddMonitoredItem(server); 코드 아래에 입력
```c
setUpEnvironment(server);
```