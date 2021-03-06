cmake_minimum_required(VERSION 2.8.3)
project(vigir_ocs_footstep_manager)

set(ROS_BUILD_TYPE Debug)
#set(ROS_BUILD_TYPE RelWithDebInfo)

## Find catkin and any catkin packages on which
## this package depends at build time
find_package(catkin REQUIRED COMPONENTS roscpp rospy tf pluginlib rviz std_msgs geometry_msgs visualization_msgs nav_msgs vigir_ocs_msgs nodelet actionlib_msgs actionlib vigir_footstep_planning_msgs vigir_footstep_planning_lib vigir_foot_pose_transformer)

## Declare things to be passed to other projects
catkin_package(
  INCLUDE_DIRS src
  LIBRARIES ${PROJECT_NAME}
        CATKIN_DEPENDS roscpp rospy tf pluginlib rviz std_msgs geometry_msgs visualization_msgs nav_msgs vigir_ocs_msgs nodelet actionlib_msgs actionlib vigir_footstep_planning_msgs vigir_footstep_planning_lib vigir_foot_pose_transformer
)

## Here we specify the list of source files, including the output of
## the previous command which is stored in ``${MOC_FILES}``.
set(SOURCE_FILES
  src/main.cpp
)

set(SOURCE_FILES_LIB
  src/footstep_manager.cpp
)

include_directories(${catkin_INCLUDE_DIRS} src)

add_library(${PROJECT_NAME} ${SOURCE_FILES_LIB})
target_link_libraries(${PROJECT_NAME} ${catkin_LIBRARIES})

add_dependencies(${PROJECT_NAME} ${catkin_EXPORTED_TARGETS})

add_executable(footstep_manager ${SOURCE_FILES})
target_link_libraries(footstep_manager ${catkin_LIBRARIES} ${PROJECT_NAME})

add_dependencies(footstep_manager ${catkin_EXPORTED_TARGETS})

install(FILES
  vigir_footstep_manager.xml
  DESTINATION ${CATKIN_PACKAGE_SHARE_DESTINATION})

install(TARGETS ${PROJECT_NAME} footstep_manager
  LIBRARY DESTINATION ${CATKIN_PACKAGE_LIB_DESTINATION}
  RUNTIME DESTINATION ${CATKIN_PACKAGE_BIN_DESTINATION}
  )
