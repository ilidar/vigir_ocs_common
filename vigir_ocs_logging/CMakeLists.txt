cmake_minimum_required(VERSION 2.8.3)
project(vigir_ocs_logging)

## Find catkin and any catkin packages on which
## this package depends at build time
find_package(catkin REQUIRED COMPONENTS roscpp rospy roslib std_msgs vigir_ocs_msgs)# vigir_perception_msgs)

## Declare things to be passed to other projects
catkin_package(
    INCLUDE_DIRS src
    LIBRARIES ${PROJECT_NAME}
    CATKIN_DEPENDS roscpp rospy roslib std_msgs vigir_ocs_msgs
)

## This plugin includes Qt widgets, so we must include Qt like so:
find_package(Qt4 COMPONENTS QtCore QtGui REQUIRED)
include(${QT_USE_FILE})

## I prefer the Qt signals and slots to avoid defining "emit", "slots",
## etc because they can conflict with boost signals, so define QT_NO_KEYWORDS here.
add_definitions(-DQT_NO_KEYWORDS)

## Here we specify which header files need to be run through "moc",
## Qt's meta-object compiler.

qt4_wrap_cpp(MOC_FILES
  src/widget.h
)


## Here we add QtCreator ui files
qt4_wrap_ui(UIC_FILES
  src/widget.ui
)

## Here we specify the list of source files, including the output of
## the previous command which is stored in ``${MOC_FILES}``.


set(SOURCE_FILES
  src/widget.cpp
  src/main.cpp
  ${MOC_FILES}
  ${UIC_FILES}
)
include_directories(include ${catkin_INCLUDE_DIRS} ${CMAKE_CURRENT_BINARY_DIR} src)

add_executable(logging_widget ${UIC_FILES} ${SOURCE_FILES})
target_link_libraries(logging_widget ${catkin_LIBRARIES} ${QT_LIBRARIES})

add_dependencies(logging_widget ${catkin_EXPORTED_TARGETS})

install( TARGETS
    logging_widget
    DESTINATION ${CATKIN_PACKAGE_BIN_DESTINATION}
)

install( DIRECTORY
    src 
    DESTINATION ${CATKIN_PACKAGE_BIN_DESTINATION}
)
install(DIRECTORY launch
  DESTINATION ${CATKIN_PACKAGE_SHARE_DESTINATION}
  FILES_MATCHING PATTERN "*.launch"
)