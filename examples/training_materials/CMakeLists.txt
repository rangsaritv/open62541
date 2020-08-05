####################
# Nodeset Examples #
####################

###################
# Custom XML      #
###################

set(FILE_NS_DIRPREFIX ${PROJECT_SOURCE_DIR}/training_materials/data_source)

if(NOT CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    # needed or cmake doesn't recognize dependencies of generated files
    set(PROJECT_BINARY_DIR ${CMAKE_BINARY_DIR})
endif()

# generate namespace from XML file
if(NOT CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    ua_generate_nodeset_and_datatypes(
        NAME "robot_model"
        FILE_NS "${FILE_NS_DIRPREFIX}/robot_model.xml"
        DEPENDS "${CMAKE_SOURCE_DIR}/tools/schema/Opc.Ua.NodeSet2.Reduced.xml"
    )
else()
    # standalone examples build expects already installed Opc.Ua.NodeSet2.Reduced.xml
    ua_generate_nodeset_and_datatypes(
        NAME "robot_model"
        FILE_NS "${FILE_NS_DIRPREFIX}/robot_model.xml"
    )
endif()

add_example(random_function data_source/random_function.c
            ${UA_NODESET_ROBOT_MODEL_SOURCES})

add_example(data_source_monitored_item monitored_item/data_source_monitored_item.c
			${UA_NODESET_ROBOT_MODEL_SOURCES})

add_example(data_source_historic_access historic_access/data_source_historic_access.c
			${UA_NODESET_ROBOT_MODEL_SOURCES})
			
add_example(data_source_alarm_condition alarm_condition/data_source_alarm_condition.c
			${UA_NODESET_ROBOT_MODEL_SOURCES})