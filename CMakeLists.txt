#--------------------------------------------------------
# The CMakeLists.txt for:                       pStartLogic
# Author(s):                              
#--------------------------------------------------------

SET(SRC
  StartLogic.cpp
  StartLogic_Info.cpp
  main.cpp
)

ADD_EXECUTABLE(pStartLogic ${SRC})

TARGET_LINK_LIBRARIES(pStartLogic
   ${MOOS_LIBRARIES}
   mbutil
   m
   pthread)

