#include <open62541/plugin/log_stdout.h>
#include <open62541/server.h>
#include <open62541/server_config_default.h>

#include <signal.h>
#include <stdlib.h>

static volatile UA_Boolean running = true;
static void stopHandler(int sig) {
    UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "received ctrl-c");
    running = false;
}

int main(void) {
    signal(SIGINT, stopHandler);
    signal(SIGTERM, stopHandler);

    UA_Server* server = UA_Server_new();
    UA_ServerConfig_setDefault(UA_Server_getConfig(server));

    UA_ObjectTypeAttributes otAttr = UA_ObjectTypeAttributes_default;
    otAttr.description = UA_LOCALIZEDTEXT("", "CompanyType");
    otAttr.displayName = UA_LOCALIZEDTEXT("", "CompanyType");
    UA_Server_addObjectTypeNode(server, UA_NODEID_NUMERIC(1, 10000),
        UA_NODEID_NUMERIC(0, UA_NS0ID_BASEOBJECTTYPE),
        UA_NODEID_NUMERIC(0, UA_NS0ID_HASSUBTYPE),
        UA_QUALIFIEDNAME(1, "CompanyType"),
        otAttr, NULL, NULL);

    UA_VariableAttributes vAttr = UA_VariableAttributes_default;
    vAttr.description = UA_LOCALIZEDTEXT("", "CompanyLocation");
    vAttr.displayName = UA_LOCALIZEDTEXT("", "Location");
    UA_String locationVar = UA_STRING("Korea");
    UA_Variant_setScalar(&vAttr.value, &locationVar, &UA_TYPES[UA_TYPES_STRING]);
    UA_Server_addVariableNode(server, UA_NODEID_NUMERIC(1, 10001),
        UA_NODEID_NUMERIC(1, 10000),
        UA_NODEID_NUMERIC(0, UA_NS0ID_HASPROPERTY),
        UA_QUALIFIEDNAME(1, "Location"),
        UA_NODEID_NULL, vAttr, NULL, NULL);

    UA_Server_addReference(server, UA_NODEID_NUMERIC(1, 10001),
    UA_NODEID_NUMERIC(0, UA_NS0ID_HASMODELLINGRULE),
    UA_EXPANDEDNODEID_NUMERIC(0, UA_NS0ID_MODELLINGRULE_MANDATORY), true);

    vAttr.description = UA_LOCALIZEDTEXT("en_US", "CompanyBusinessField");
    vAttr.displayName = UA_LOCALIZEDTEXT("en_US", "businessField");
    UA_String businessField = UA_STRING("R&D");
    UA_Variant_setScalarCopy(&vAttr.value, &businessField, &UA_TYPES[UA_TYPES_STRING]);
    UA_Server_addVariableNode(server, UA_NODEID_NUMERIC(1, 10002),
        UA_NODEID_NUMERIC(1, 10000),
        UA_NODEID_NUMERIC(0, UA_NS0ID_HASPROPERTY),
        UA_QUALIFIEDNAME(1, "businessField"),
        UA_NODEID_NULL, vAttr, NULL, NULL);


    UA_Server_addReference(server, UA_NODEID_NUMERIC(1, 10002),
    UA_NODEID_NUMERIC(0, UA_NS0ID_HASMODELLINGRULE),
    UA_EXPANDEDNODEID_NUMERIC(0, UA_NS0ID_MODELLINGRULE_MANDATORY), true);


    UA_ObjectAttributes oAttr = UA_ObjectAttributes_default;
    oAttr.description = UA_LOCALIZEDTEXT("en_US", "InheritanceCase");
    oAttr.displayName = UA_LOCALIZEDTEXT("en_US", "TUV SUD");
    UA_Server_addObjectNode(server, UA_NODEID_NUMERIC(1, 0),
        UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER),
        UA_NODEID_NUMERIC(0, UA_NS0ID_ORGANIZES),
        UA_QUALIFIEDNAME(1, "Company"),
        UA_NODEID_NUMERIC(1, 10000),
        oAttr, NULL, NULL);


    UA_StatusCode retval = UA_Server_run(server, &running);

    UA_Server_delete(server);
    return retval == UA_STATUSCODE_GOOD ? EXIT_SUCCESS : EXIT_FAILURE;
}
