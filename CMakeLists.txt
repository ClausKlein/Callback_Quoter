cmake_minimum_required(VERSION 3.21...3.27)

option(CMAKE_EXPORT_COMPILE_COMMANDS "for clang-tidy" YES)

#================================
project(idl2cpp VERSION 0.1.0 LANGUAGES CXX)
#================================

## require a C++ standard
if(NOT DEFINED CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 17)
  option(CMAKE_CXX_EXTENSIONS "" NO)
  option(CMAKE_CXX_STANDARD_REQUIRED "" YES)
endif()

if(NOT TARGET Axcioma::itaox11)
  find_package(itaox11 CONFIG REQUIRED)
endif()

include(GenerateTaoIdl)
tao_wrap_idl(Consumer.idl Notifier.idl)

add_library(idl2cpp STATIC ${TAO_IDL_GENERATED})
target_link_libraries(idl2cpp PUBLIC Axcioma::itaox11)
set_target_properties(idl2cpp PROPERTIES CXX_STANDARD 17)

if(APPLE) # "think different", indeed
  target_compile_definitions(
    idl2cpp PUBLIC __ACE_INLINE__ ACE_HAS_IPV6 ACE_HAS_VERSIONED_NAMESPACE=1 ACE_HAS_CUSTOM_EXPORT_MACROS=0
  )
  target_compile_options(idl2cpp PUBLIC -fdelayed-template-parsing)
endif()

add_executable(
  notifier
  # NotifierC.cpp NotifierS.cpp ConsumerC.cpp
  Notifier_i.cpp Notifier_Input_Handler.cpp notifier.cpp
)
target_link_libraries(notifier PUBLIC idl2cpp)

add_executable(
  consumer
  # ConsumerC.cpp ConsumerS.cpp NotifierC.cpp
  Consumer_Input_Handler.cpp
  Consumer_Signal_Handler.cpp
  Consumer_Handler.cpp
  consumer.cpp
  Consumer_i.cpp
)
target_link_libraries(consumer PUBLIC idl2cpp)

add_executable(
  supplier
  # ConsumerC.cpp ConsumerS.cpp NotifierC.cpp
  Supplier_i.cpp Supplier_Timer_Handler.cpp supplier.cpp Consumer_i.cpp
)
target_link_libraries(supplier PUBLIC idl2cpp)

if(CMAKE_SKIP_INSTALL_RULES)
  return()
endif()

install(TARGETS ${PROJECT_NAME} notifier consumer supplier RUNTIME COMPONENT ${PROJECT_NAME})

install(IMPORTED_RUNTIME_ARTIFACTS notifier RUNTIME_DEPENDENCY_SET _dependency_set)

# FIXME: CONFLICTING_DEPENDENCIES_PREFIX  _conflicts
# cmake-format: off
install(
  RUNTIME_DEPENDENCY_SET _dependency_set
  POST_EXCLUDE_REGEXES ${CMAKE_SOURCE_DIR}/lib/
  POST_EXCLUDE_REGEXES ${CMAKE_SOURCE_DIR}/ACE/ACE/lib/
  POST_EXCLUDE_REGEXES "${CMAKE_INSTALL_PREFIX}/lib"
  RUNTIME DESTINATION lib
)
# cmake-format: on
