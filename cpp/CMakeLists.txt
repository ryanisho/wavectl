cmake_minimum_required(VERSION 3.18)
project(audio_engine LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

find_package(pybind11 CONFIG REQUIRED)

find_path(RTAUDIO_INCLUDE_DIR rtaudio/RtAudio.h)
find_library(RTAUDIO_LIBRARY NAMES rtaudio)
if(NOT RTAUDIO_INCLUDE_DIR OR NOT RTAUDIO_LIBRARY)
  message(FATAL_ERROR "RtAudio not found. brew install rtaudio")
endif()

pybind11_add_module(audio_engine audio_engine.cpp)
target_include_directories(audio_engine PRIVATE ${RTAUDIO_INCLUDE_DIR})
target_link_libraries(audio_engine PRIVATE ${RTAUDIO_LIBRARY})

if(APPLE)
  find_library(COREAUDIO_LIBRARY CoreAudio)
  find_library(AUDIOTOOLBOX_LIBRARY AudioToolbox)
  find_library(AUDIOUNIT_LIBRARY AudioUnit)
  target_link_libraries(audio_engine PRIVATE ${COREAUDIO_LIBRARY} ${AUDIOUNIT_LIBRARY} ${AUDIOTOOLBOX_LIBRARY})
endif()
