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


    UA_ObjectAttributes  oAttr = UA_ObjectAttributes_default;
    oAttr.description = UA_LOCALIZEDTEXT("en_US", "InheritanceCase");
    oAttr.displayName = UA_LOCALIZEDTEXT("en_US", "TUV SUD");
    UA_Server_addObjectNode(server, UA_NODEID_NUMERIC(1, 0),
        UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER),
        UA_NODEID_NUMERIC(0, UA_NS0ID_ORGANIZES),
        UA_QUALIFIEDNAME(1, "Company"),
        UA_NODEID_NUMERIC(1, 10000),
        oAttr, NULL, NULL);

    otAttr = UA_ObjectTypeAttributes_default;
    otAttr.description = UA_LOCALIZEDTEXT("", "SensorType");
    otAttr.displayName = UA_LOCALIZEDTEXT("", "SensorType");
    UA_Server_addObjectTypeNode(server, UA_NODEID_NUMERIC(1, 20000),
        UA_NODEID_NUMERIC(0, UA_NS0ID_BASEOBJECTTYPE),
        UA_NODEID_NUMERIC(0, UA_NS0ID_HASSUBTYPE),
        UA_QUALIFIEDNAME(1, "SnsorType"),
        otAttr, NULL, NULL);

    vAttr = UA_VariableAttributes_default;
    vAttr.description = UA_LOCALIZEDTEXT("", "MesauredValue");
    vAttr.displayName = UA_LOCALIZEDTEXT("", "MeasuredValue");
    UA_Int16 measuredVar = 0;
    UA_Variant_setScalar(&vAttr.value, &measuredVar, &UA_TYPES[UA_TYPES_INT16]);
    UA_Server_addVariableNode(server, UA_NODEID_NUMERIC(1, 20001),
        UA_NODEID_NUMERIC(1, 20000),
        UA_NODEID_NUMERIC(0, UA_NS0ID_HASPROPERTY),
        UA_QUALIFIEDNAME(1, "MeasuredValue"),
        UA_NODEID_NULL, vAttr, NULL, NULL);


    UA_Server_addReference(server, UA_NODEID_NUMERIC(1, 20001),
    UA_NODEID_NUMERIC(0, UA_NS0ID_HASMODELLINGRULE),
    UA_EXPANDEDNODEID_NUMERIC(0, UA_NS0ID_MODELLINGRULE_MANDATORY), true);

    vAttr.description = UA_LOCALIZEDTEXT("en_US", "Status");
    vAttr.displayName = UA_LOCALIZEDTEXT("en_US", "Status");
    UA_Boolean robotStatus = false;
    UA_Variant_setScalarCopy(&vAttr.value, &robotStatus, &UA_TYPES[UA_TYPES_BOOLEAN]);
    UA_Server_addVariableNode(server, UA_NODEID_NUMERIC(1, 20002),
        UA_NODEID_NUMERIC(1, 20000),
        UA_NODEID_NUMERIC(0, UA_NS0ID_HASPROPERTY),
        UA_QUALIFIEDNAME(1, "Status"),
        UA_NODEID_NULL, vAttr, NULL, NULL);


    UA_Server_addReference(server, UA_NODEID_NUMERIC(1, 20002),
    UA_NODEID_NUMERIC(0, UA_NS0ID_HASMODELLINGRULE),
    UA_EXPANDEDNODEID_NUMERIC(0, UA_NS0ID_MODELLINGRULE_MANDATORY), true);


    otAttr = UA_ObjectTypeAttributes_default;
    otAttr.description = UA_LOCALIZEDTEXT("", "RobotType");
    otAttr.displayName = UA_LOCALIZEDTEXT("", "RobotType");
    UA_Server_addObjectTypeNode(server, UA_NODEID_NUMERIC(1, 30000),
        UA_NODEID_NUMERIC(0, UA_NS0ID_BASEOBJECTTYPE),
        UA_NODEID_NUMERIC(0, UA_NS0ID_HASSUBTYPE),
        UA_QUALIFIEDNAME(1, "RobotType"),
        otAttr, NULL, NULL);


    vAttr = UA_VariableAttributes_default;
    vAttr.description = UA_LOCALIZEDTEXT("", "DoF");
    vAttr.displayName = UA_LOCALIZEDTEXT("", "DoF");
    UA_Int16 dof = 3;
    UA_Variant_setScalar(&vAttr.value, &dof, &UA_TYPES[UA_TYPES_INT16]);
    UA_Server_addVariableNode(server, UA_NODEID_NUMERIC(1, 30001),
        UA_NODEID_NUMERIC(1, 30000),
        UA_NODEID_NUMERIC(0, UA_NS0ID_HASPROPERTY),
        UA_QUALIFIEDNAME(1, "DoF"),
        UA_NODEID_NULL, vAttr, NULL, NULL);


    UA_Server_addReference(server, UA_NODEID_NUMERIC(1, 30001),
    UA_NODEID_NUMERIC(0, UA_NS0ID_HASMODELLINGRULE),
    UA_EXPANDEDNODEID_NUMERIC(0, UA_NS0ID_MODELLINGRULE_MANDATORY), true);


    vAttr.description = UA_LOCALIZEDTEXT("en_US", "Color");
    vAttr.displayName = UA_LOCALIZEDTEXT("en_US", "Color");
    UA_String color = UA_STRING("Red");
    UA_Variant_setScalarCopy(&vAttr.value, &color, &UA_TYPES[UA_TYPES_STRING]);
    UA_Server_addVariableNode(server, UA_NODEID_NUMERIC(1, 30002),
       UA_NODEID_NUMERIC(1, 30000),
       UA_NODEID_NUMERIC(0, UA_NS0ID_HASPROPERTY),
       UA_QUALIFIEDNAME(1, "Color"),
       UA_NODEID_NULL, vAttr, NULL, NULL);

    
    UA_Server_addReference(server, UA_NODEID_NUMERIC(1, 30002),
    UA_NODEID_NUMERIC(0, UA_NS0ID_HASMODELLINGRULE),
    UA_EXPANDEDNODEID_NUMERIC(0, UA_NS0ID_MODELLINGRULE_MANDATORY), true);

    oAttr = UA_ObjectAttributes_default;
    oAttr.description = UA_LOCALIZEDTEXT("en_US", "Robot");
    oAttr.displayName = UA_LOCALIZEDTEXT("en_US", "Robot");
    UA_Server_addObjectNode(server, UA_NODEID_NUMERIC(1, 40000),
        UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER),
        UA_NODEID_NUMERIC(0, UA_NS0ID_ORGANIZES),
        UA_QUALIFIEDNAME(1, "Robot"),
        UA_NODEID_NUMERIC(1, 30000),
        oAttr, NULL, NULL);

    oAttr = UA_ObjectAttributes_default;
    oAttr.description = UA_LOCALIZEDTEXT("en_US", "SpeedSensor");
    oAttr.displayName = UA_LOCALIZEDTEXT("en_US", "SpeedSensor");
    UA_Server_addObjectNode(server, UA_NODEID_NUMERIC(1, 40001),
        UA_NODEID_NUMERIC(1, 40000),
        UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT),
        UA_QUALIFIEDNAME(1, "SpeedSensor"),
        UA_NODEID_NUMERIC(1, 20000),
        oAttr, NULL, NULL);

    oAttr = UA_ObjectAttributes_default;
    oAttr.description = UA_LOCALIZEDTEXT("en_US", "TempSensor");
    oAttr.displayName = UA_LOCALIZEDTEXT("en_US", "TempSensor");
    UA_Server_addObjectNode(server, UA_NODEID_NUMERIC(1, 40002),
        UA_NODEID_NUMERIC(1, 40000),
        UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT),
        UA_QUALIFIEDNAME(1, "TempSensor"),
        UA_NODEID_NUMERIC(1, 20000),
        oAttr, NULL, NULL);



    UA_StatusCode retval = UA_Server_run(server, &running);

    UA_Server_delete(server);
    return retval == UA_STATUSCODE_GOOD ? EXIT_SUCCESS : EXIT_FAILURE;
}
