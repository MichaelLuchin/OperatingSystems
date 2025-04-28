cmake_minimum_required(VERSION 3.30)
project(OperatingSystems)

set(CMAKE_C_COMPILER "gcc")
set(CMAKE_CXX_COMPILER "g++")

# Настройки компилятора
set(CMAKE_CXX_STANDARD 17)
add_compile_options(-Wall -Wextra -pedantic)

# Общие определения для Windows
if(WIN32)
    add_definitions(-D_WIN32_WINNT=0x0601 -DWIN32_LEAN_AND_MEAN)
endif()

# Сервер
add_executable(CalculatorServer server/Server.cpp)
if(WIN32)
    target_link_libraries(CalculatorServer ws2_32)
endif()

# Клиент
add_executable(CalculatorClient client/Client.cpp)
if(WIN32)
    target_link_libraries(CalculatorClient ws2_32)
endif()