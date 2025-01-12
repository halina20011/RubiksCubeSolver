set(requires "")
idf_build_get_property(target IDF_TARGET)

if(${target} STREQUAL "linux")
    list(APPEND requires protocol_examples_common nvs_flash)
endif()

idf_component_register(SRCS 
    "main.c"
    "./motor.c"
    "./sensor.c"
    "./wifi.c"
    "./mcode.c"
    INCLUDE_DIRS "."
    REQUIRES ${requires})
