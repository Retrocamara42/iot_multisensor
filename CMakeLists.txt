# The following lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.5)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(iot-multisensor)

target_add_binary_data(${CMAKE_PROJECT_NAME}.elf "/main/credentials/AmazonRootCA1.pem" TEXT)
target_add_binary_data(${CMAKE_PROJECT_NAME}.elf "/main/credentials/iot_multisensor_certificate.pem.crt" TEXT)
target_add_binary_data(${CMAKE_PROJECT_NAME}.elf "/main/credentials/iot_multisensor_private.pem.key" TEXT)
