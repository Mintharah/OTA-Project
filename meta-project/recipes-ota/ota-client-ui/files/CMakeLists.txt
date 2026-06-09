cmake_minimum_required(VERSION 3.16)
project(OTAUpdater VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Quick)
qt_standard_project_setup()
qt_policy(SET QTP0001 NEW)
qt_policy(SET QTP0004 NEW)

qt_add_executable(appOTAUpdater
    main.cpp
    OTAClientController.h
    OTAClientController.cpp
    src-gen/v1/com/ota/OTAFlashServiceSomeIPProxy.cpp
    src-gen/v1/com/ota/OTAFlashServiceSomeIPDeployment.cpp
)

qt_add_qml_module(appOTAUpdater
    URI OTAUpdater
    VERSION 1.0
    QML_FILES
        Main.qml
        WaitingView.qml
        ReceivingView.qml
        ResultView.qml
)

qt_add_resources(appOTAUpdater "gifs"
    PREFIX "/images"
    FILES
        images/loading_melody.gif
        images/success_melody.gif
        images/sleep_melody.gif
        images/error_melody.gif
)

target_include_directories(appOTAUpdater PRIVATE
    ${CMAKE_SYSROOT}/usr/include/CommonAPI-3.2
    ${CMAKE_SOURCE_DIR}/src-gen
)

target_link_libraries(appOTAUpdater
    PRIVATE
    Qt6::Quick
    CommonAPI
    CommonAPI-SomeIP
    vsomeip3
)

target_link_options(appOTAUpdater PRIVATE -Wl,--no-as-needed)
