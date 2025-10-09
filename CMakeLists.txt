cmake_minimum_required(VERSION 3.25...4.1)

option(CMAKE_EXPORT_COMPILE_COMMANDS "for clang-tidy" YES)

#===========================================
project(idl2cpp VERSION 0.1.2 LANGUAGES CXX)
#===========================================

## require a C++ standard
if(NOT DEFINED CMAKE_CXX_STANDARD)
    set(CMAKE_CXX_STANDARD 17)
endif()
option(CMAKE_CXX_EXTENSIONS "" NO)
option(CMAKE_CXX_STANDARD_REQUIRED "" YES)

if(UNIX AND CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    set(CMAKE_DEPFILE_FLAGS_C "-MMD -MT <DEP_TARGET> -MF <DEP_FILE>")
    set(CMAKE_DEPFILE_FLAGS_CXX "-MMD -MT <DEP_TARGET> -MF <DEP_FILE>")
endif()

if(NOT TARGET Axcioma::itaox11)
    find_package(itaox11 CONFIG REQUIRED)
    list(APPEND CMAKE_MODULE_PATH ${CMAKE_STAGING_PREFIX}/share/cmake/modules)
endif()

include(GenerateTaoIdl)
tao_wrap_idl(Consumer.idl Notifier.idl)

add_library(idl2cpp ${TAO_IDL_GENERATED})
target_link_libraries(idl2cpp PUBLIC Axcioma::itaox11)
target_compile_features(idl2cpp INTERFACE cxx_std_17)

add_executable(
    notifier
    # NotifierC.cpp NotifierS.cpp ConsumerC.cpp
    Notifier_i.cpp
    Notifier_Input_Handler.cpp
    notifier.cpp
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
    Supplier_i.cpp
    Supplier_Timer_Handler.cpp
    supplier.cpp
    Consumer_i.cpp
)
target_link_libraries(supplier PUBLIC idl2cpp)

if(CMAKE_SKIP_INSTALL_RULES)
    return()
endif()

install(
    TARGETS ${PROJECT_NAME} notifier consumer supplier
    RUNTIME COMPONENT ${PROJECT_NAME}
)

if(APPLE) # "think different", indeed
    install(
        IMPORTED_RUNTIME_ARTIFACTS notifier
        RUNTIME_DEPENDENCY_SET _dependency_set
    )
    # FIXME: CONFLICTING_DEPENDENCIES_PREFIX  _conflicts
    install(
        RUNTIME_DEPENDENCY_SET _dependency_set
        POST_EXCLUDE_REGEXES ${CMAKE_SOURCE_DIR}/lib
        POST_EXCLUDE_REGEXES ${CMAKE_SOURCE_DIR}/ACE/ACE/lib
        POST_EXCLUDE_REGEXES ${CMAKE_STAGING_PREFIX}/lib
        RUNTIME DESTINATION lib
    )
endif()

enable_testing()

#
# NOTE: this test works only after install and with taox11 environment! CK
#
install(FILES example.stocks DESTINATION bin)
install(PROGRAMS run_test.pl DESTINATION bin)
add_test(
    NAME example
    COMMAND perl -I${CMAKE_STAGING_PREFIX}/share/perl5 run_test.pl -s
    WORKING_DIRECTORY ${CMAKE_STAGING_PREFIX}/bin
)

set(CPACK_GENERATOR "TGZ")
include(CPack)
