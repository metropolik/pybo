include_directories (${PROJECT_SOURCE_DIR}/src/dep/include ${PROJECT_SOURCE_DIR}/src/shared ${PROJECT_SOURCE_DIR}/src/Client)

set(PSEUWOW_LIBS
shared zlib ${OPENSSL_LIBRARIES} ${OPENSSL_EXTRA_LIBRARIES}
)

link_directories (${CMAKE_INSTALL_PREFIX})
if(WIN32)
  link_directories (${PROJECT_SOURCE_DIR}/src/dep/lib/static)
  set(PSEUWOW_LIBS ${PSEUWOW_LIBS} Winmm)
endif()
if(UNIX)
  set(EXECUTABLE_LINK_FLAGS "-pthread" ${EXECUTABLE_LINK_FLAGS})
  set(PSEUWOW_LIBS ${PSEUWOW_LIBS} GL Xxf86vm bz2)
endif()

#python
find_package(PythonLibs REQUIRED)
include_directories(${PYTHON_INCLUDE_DIRS})
SET(PSEUWOW_LIBS "${PSEUWOW_LIBS} ${PYTHON_LIBRARIES}")

add_executable (pseuwow
Realm/RealmSession.cpp
Realm/RealmSocket.cpp

World/Bag.cpp
World/CacheHandler.cpp
World/Channel.cpp
World/CMSGConstructor.cpp
World/Corpse.cpp
World/DynamicObject.cpp
World/GameObject.cpp
World/Item.cpp
World/MovementMgr.cpp
World/Object.cpp
World/ObjMgr.cpp
World/Opcodes.cpp
World/Player.cpp
World/Unit.cpp
World/UpdateData.cpp
World/UpdateFields.cpp
World/World.cpp
World/WorldPacket.cpp
World/WorldSession.cpp
World/WorldSocket.cpp

World/PyBehaviour.cpp

main.cpp
PseuWoW.cpp
)



# Link the executable to the libraries.
target_link_libraries (pseuwow ${PSEUWOW_LIBS})

set_target_properties(pseuwow PROPERTIES LINK_FLAGS
  "${EXECUTABLE_LINK_FLAGS}"
)
install(TARGETS pseuwow DESTINATION ${CMAKE_INSTALL_PREFIX})
