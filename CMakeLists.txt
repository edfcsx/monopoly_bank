# CMakeList.txt: projeto de CMake para monopoly_bank, incluir origem e definir
# lógica específica a um projeto aqui.
#
cmake_minimum_required (VERSION 3.8)

# Habilite a Recarga Dinâmica para compiladores MSVC, se houver suporte.
if (POLICY CMP0141)
    cmake_policy(SET CMP0141 NEW)
    set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project ("monopoly_bank")

#adiciona as varíaveis de ambiente para o uso do Boost
set(BOOST_ROOT "C:/Sdks/boost_1_84_0")

set(Boost_USE_STATIC_LIBS    ON)
set(Boost_USE_DEBUG_LIBS     OFF)
set(Boost_USE_MULTITHREADED  ON)
set(Boost_USE_STATIC_RUNTIME OFF)

find_package(Boost)
find_package(OpenSSL REQUIRED)

# fim do uso do Boost

# variaveis da biblioteca ws2_32 para o uso de sockets
set(USE_WS2_32 OFF)

if(WIN32 AND USE_WS2_32)
    target_link_libraries(${PROJECT_NAME} PRIVATE ws2_32)
endif()

if(NOT Boost_FOUND)
    message(FATAL_ERROR "Boost not found.")
endif()

# Adicione a origem ao executável deste projeto.
add_executable (${PROJECT_NAME}
        networking.h
        server.cpp
        server.h
        types.h
        main.cpp
        connection.cpp
        connection.h
        player.cpp
        player.h
        json.hpp
        icommand.h
        ping_command.h
        transfer_command.h
        authenticate_command.h
        connection_manager.cpp
        connection_manager.h
        websocket_handshake.cpp
        websocket_handshake.h
        uuid.h
)

if (CMAKE_VERSION VERSION_GREATER 3.12)
    set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 20)
endif()

# TODO: Adicione testes e instale destinos, se necessário.
target_include_directories(${PROJECT_NAME} PRIVATE ${Boost_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} PRIVATE ${Boost_LIBRARIES} OpenSSL::SSL OpenSSL::Crypto)
