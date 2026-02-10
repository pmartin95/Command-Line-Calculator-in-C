find_package(Qt6 REQUIRED COMPONENTS Widgets)
qt_standard_project_setup()

set(GUI_SOURCES
    main.cpp
    MainWindow.cpp
    MainWindow.h
    CalcEngine.cpp
    CalcEngine.h
    HistoryModel.cpp
    HistoryModel.h
)

add_executable(calculator-gui ${GUI_SOURCES})

target_link_libraries(calculator-gui PRIVATE
    calc_core
    Qt6::Widgets
)

target_include_directories(calculator-gui PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)
