cmake_minimum_required(VERSION 3.16)
project(Application VERSION 0.1 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Core Quick SerialPort DBus Sql)

qt_standard_project_setup(REQUIRES 6.8)

qt_add_executable(appApplication
    main.cpp
)

qt_add_qml_module(appApplication
    URI Application
    QML_FILES
        Main.qml
        HomePage.qml
        RadioSourceView.qml
        CarplayPage.qml
        CDView.qml
        PhonePage.qml
        SettingsPage.qml
        CANBusPage.qml
        CANRawDataPage.qml
        ClimatePage.qml
        AmbientLightingPage.qml
        BluetoothSettingsPage.qml
        IncomingCallOverlay.qml
    SOURCES
        uartController.cpp
        uartController.h
        carplayController.h
        carplayController.cpp
        canController.h
        canController.cpp
        ambientController.h
        ambientController.cpp
        BluetoothManager.h
        BluetoothManager.cpp
        CallManager.h
        CallManager.cpp
        ContactsManager.h
        ContactsManager.cpp
        QML_FILES ActiveCallPage.qml
        QML_FILES MediaPage.qml
        QML_FILES BluetoothMusicView.qml
        QML_FILES AuxView.qml
        SOURCES BluetoothMediaPlayer.cpp
        SOURCES BluetoothMediaPlayer.h
)

set_target_properties(appApplication PROPERTIES
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

target_link_libraries(appApplication
    PRIVATE Qt6::Core
    PRIVATE Qt6::Quick
    PRIVATE Qt6::SerialPort
    PRIVATE Qt6::DBus
    PRIVATE Qt6::Sql
)

include(GNUInstallDirs)
install(TARGETS appApplication
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
