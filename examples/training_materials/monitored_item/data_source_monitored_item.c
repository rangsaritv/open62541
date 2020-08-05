/* This work is licensed under a Creative Commons CCZero 1.0 Universal License.
 * See http://creativecommons.org/publicdomain/zero/1.0/ for more information. */

#include <open62541/plugin/log_stdout.h>
#include <open62541/server.h>
#include <open62541/server_config_default.h>
#include <open62541/client_subscriptions.h>

#include "open62541/namespace_robot_model_generated.h"

#include <signal.h>
#include <stdlib.h>

UA_Boolean running = true;

static void stopHandler(int sign) {
    UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_SERVER, "received ctrl-c");
    running = false;
}
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
    UA_NodeId MonitoredItemNodeId = UA_NODEID_NUMERIC(2, 6010);
    UA_MonitoredItemCreateRequest monRequest =
        UA_MonitoredItemCreateRequest_default(MonitoredItemNodeId);
    monRequest.requestedParameters.samplingInterval = 3000.0; /*
    1 s interval */
    UA_Server_createDataChangeMonitoredItem(server, UA_TIMESTAMPSTORETURN_SOURCE,
        monRequest, NULL, dataChangeNotificationCallback);
}
int main(int argc, char** argv) {
    signal(SIGINT, stopHandler);
    signal(SIGTERM, stopHandler);
    
    UA_Server *server = UA_Server_new();
    UA_ServerConfig_setDefault(UA_Server_getConfig(server));

    UA_StatusCode retval;
    /* create nodes from nodeset */
    if(namespace_robot_model_generated(server) != UA_STATUSCODE_GOOD) {
        UA_LOG_ERROR(UA_Log_Stdout, UA_LOGCATEGORY_SERVER, "Could not add the example nodeset. "
        "Check previous output for any error.");
        retval = UA_STATUSCODE_BADUNEXPECTEDERROR;
    } else {

        UA_Server_addRepeatedCallback(server, writeRandomVariable, NULL, 5000, NULL);
        addMonitoredItem(server);
        
        retval = UA_Server_run(server, &running);
    }

    UA_Server_delete(server);
    return retval == UA_STATUSCODE_GOOD ? EXIT_SUCCESS : EXIT_FAILURE;
}
