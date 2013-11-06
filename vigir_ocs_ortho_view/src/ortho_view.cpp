/*
 * OrthoView class implementation.
 *
 * Author: Felipe Bacim.
 *
 * Based on librviz_tutorials.
 *
 * Latest changes (12/08/2012):
 */

#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>

#include <OGRE/OgreRenderWindow.h>

#include "rviz/visualization_manager.h"
#include "rviz/view_controller.h"
#include "rviz/tool_manager.h"
#include "rviz/view_manager.h"
#include "rviz/display.h"
#include "ortho_view_controller_custom.h"
#include <render_panel_custom.h>

#include <flor_perception_msgs/EnvironmentRegionRequest.h>

#include "ortho_view.h"

namespace vigir_ocs
{
// Constructor for OrthoView.  This does most of the work of the class.
OrthoView::OrthoView( QWidget* parent, rviz::VisualizationManager* context )
    : Base3DView( context, "/world", parent )
    , setting_pose_(false)
{
    // block sending left/right mouse events to rviz by default
    //((rviz::RenderPanelCustom*)render_panel_)->setEventFilters(rviz::RenderPanelCustom::MOUSE_PRESS_EVENT,false,Qt::NoModifier,Qt::LeftButton | Qt::RightButton);
    //((rviz::RenderPanelCustom*)render_panel_)->setEventFilters(rviz::RenderPanelCustom::MOUSE_RELEASE_EVENT,false,Qt::NoModifier,Qt::LeftButton | Qt::RightButton);
    //((rviz::RenderPanelCustom*)render_panel_)->setEventFilters(rviz::RenderPanelCustom::MOUSE_MOVE_EVENT,false,Qt::NoModifier,Qt::LeftButton | Qt::RightButton);
    //((rviz::RenderPanelCustom*)render_panel_)->setEventFilters(rviz::RenderPanelCustom::KEY_PRESS_EVENT,true,Qt::NoModifier,Qt::NoButton);

    // set the camera to be topdownortho
    //rviz::ViewManager* view_man_ = manager_->getViewManager();
    ortho_view_controller_ = new rviz::OrthoViewControllerCustom();//view_man_->create( "rviz/OrthoViewControllerCustom" );
    //view_man_->setCurrentFrom( ortho_view_controller_ );
    //ortho_view_controller_->initialize(context);
    ((rviz::OrthoViewControllerCustom*)ortho_view_controller_)->initialize(context, render_panel_);
    render_panel_->setViewController(ortho_view_controller_);

    // Add support for selection
    //selection_tool_ = manager_->getToolManager()->addTool( "rviz/ImageSelectionToolCustom" );
    //QObject::connect(mouse_event_handler_, SIGNAL(mouseLeftButton(bool,int,int)), this, SLOT(enableSelectionTool(bool,int,int)));
    //QObject::connect(this, SIGNAL(unHighlight()), selection_tool_, SLOT(unHighlight()));

    Q_EMIT unHighlight();

    // create publisher for grid map
    grid_map_request_pub_ = nh_.advertise<flor_perception_msgs::EnvironmentRegionRequest>( "/flor/worldmodel/ocs/gridmap_request", 1, false );

    // create publisher for grid map
    octomap_request_pub_ = nh_.advertise<flor_perception_msgs::EnvironmentRegionRequest>( "/flor/worldmodel/ocs/octomap_request", 1, false );

    // connect to selection display to query position/raycast
    //QObject::connect(this, SIGNAL(queryPosition(int,int,Ogre::Vector3&)), selection_3d_display_, SLOT(queryPosition(int,int,Ogre::Vector3&)));

    // subscribe to goal pose so we can add filters back
    set_walk_goal_sub_ = nh_.subscribe<geometry_msgs::PoseStamped>( "/goal_pose_walk", 5, &OrthoView::processGoalPose, this );
    set_step_goal_sub_ = nh_.subscribe<geometry_msgs::PoseStamped>( "/goal_pose_step", 5, &OrthoView::processGoalPose, this );

    // make sure we're still able to cancel set goal pose
    QObject::connect(render_panel_, SIGNAL(signalKeyPressEvent(QKeyEvent*)), this, SLOT(keyPressEvent(QKeyEvent*)));

    //render_panel_->getCamera()->setPosition(0,5,0);
}

// Destructor.
OrthoView::~OrthoView()
{
}

void OrthoView::timerEvent(QTimerEvent *event)
{
    // call the base3dview version of the timerevent
    Base3DView::timerEvent(event);

    ortho_view_controller_->update(0,0);

//    float lastFPS, avgFPS, bestFPS, worstFPS;

//    render_panel_->getRenderWindow()->getStatistics( lastFPS, avgFPS, bestFPS, worstFPS );
//    std::cout << "Camera (" << /*ortho_view_controller_->subProp( "View Plane" )->getValue(). <<*/ "): " << lastFPS << ", " << avgFPS << ", " << bestFPS << ", " << worstFPS << std::endl;

    //std::cout << "camera position: " << render_panel_->getCamera()->getPosition().x << ", " << render_panel_->getCamera()->getPosition().z << ", " << render_panel_->getCamera()->getPosition().z << std::endl;
    //std::cout << "camera direction: " << render_panel_->getCamera()->getDirection().x << ", " << render_panel_->getCamera()->getDirection().z << ", " << render_panel_->getCamera()->getDirection().z << std::endl;
}

void OrthoView::setViewPlane(const QString& view_plane)
{
    //rviz::ViewManager* view_man_ = manager_->getViewManager();
    ortho_view_controller_->subProp( "View Plane" )->setValue( view_plane.toStdString().c_str() );
    //view_man_->setCurrentFrom( ortho_view_controller_ );

    // set the camera position
    //if("XY")
    //render_panel_->getCamera()->setPosition(0,0,5);
    //render_panel_->getCamera()->setDirection(0,0,-1);
}

void OrthoView::enableSelectionTool(bool activate, int x, int y)
{
    if(!setting_pose_)
    {
        std::cout << "selection tool: " << activate << std::endl;
        if(activate)
        {
            selected_area_[0] = x;
            selected_area_[1] = y;
            // change to the selection tool and unblock events
            manager_->getToolManager()->setCurrentTool( selection_tool_ );
            ((rviz::RenderPanelCustom*)render_panel_)->setEventFilters(rviz::RenderPanelCustom::MOUSE_PRESS_EVENT,false,Qt::NoModifier,Qt::RightButton);
            ((rviz::RenderPanelCustom*)render_panel_)->setEventFilters(rviz::RenderPanelCustom::MOUSE_RELEASE_EVENT,false,Qt::NoModifier,Qt::RightButton);
            ((rviz::RenderPanelCustom*)render_panel_)->setEventFilters(rviz::RenderPanelCustom::MOUSE_MOVE_EVENT,false,Qt::NoModifier,Qt::RightButton);
        }
        else
        {
            selected_area_[2] = x;
            selected_area_[3] = y;
            // block again and change back
            ((rviz::RenderPanelCustom*)render_panel_)->setEventFilters(rviz::RenderPanelCustom::MOUSE_PRESS_EVENT,false,Qt::NoModifier,Qt::LeftButton | Qt::RightButton);
            ((rviz::RenderPanelCustom*)render_panel_)->setEventFilters(rviz::RenderPanelCustom::MOUSE_RELEASE_EVENT,false,Qt::NoModifier,Qt::LeftButton | Qt::RightButton);
            ((rviz::RenderPanelCustom*)render_panel_)->setEventFilters(rviz::RenderPanelCustom::MOUSE_MOVE_EVENT,false,Qt::NoModifier,Qt::LeftButton | Qt::RightButton);
            manager_->getToolManager()->setCurrentTool( move_camera_tool_ );

        }
    }
}

void OrthoView::requestMap(double min_z, double max_z, double resolution)
{
    float win_width = render_panel_->width();
    float win_height = render_panel_->height();

    Ogre::Vector3 min, max;
    Q_EMIT queryPosition(selected_area_[0],selected_area_[1],min);
    Q_EMIT queryPosition(selected_area_[2],selected_area_[3],max);

    flor_perception_msgs::EnvironmentRegionRequest cmd;

    cmd.bounding_box_min.x = min.x;
    cmd.bounding_box_min.y = min.y;
    cmd.bounding_box_min.z = min_z;

    cmd.bounding_box_max.x = max.x;
    cmd.bounding_box_max.y = max.y;
    cmd.bounding_box_max.z = max_z;

    cmd.resolution = resolution;

    grid_map_request_pub_.publish(cmd);

    Q_EMIT unHighlight();
}

void OrthoView::requestOctomap(double min_z, double max_z, double resolution)
{
    float win_width = render_panel_->width();
    float win_height = render_panel_->height();

    Ogre::Vector3 min, max;
    Q_EMIT queryPosition(selected_area_[0],selected_area_[1],min);
    Q_EMIT queryPosition(selected_area_[2],selected_area_[3],max);

    flor_perception_msgs::EnvironmentRegionRequest cmd;

    cmd.bounding_box_min.x = min.x;
    cmd.bounding_box_min.y = min.y;
    cmd.bounding_box_min.z = min_z;

    cmd.bounding_box_max.x = max.x;
    cmd.bounding_box_max.y = max.y;
    cmd.bounding_box_max.z = max_z;

    cmd.resolution = resolution;

    octomap_request_pub_.publish(cmd);

    Q_EMIT unHighlight();
}

void OrthoView::defineWalkPosePressed()
{
    //ROS_ERROR("vector pressed in map");
    ((rviz::RenderPanelCustom*)render_panel_)->setEventFilters(rviz::RenderPanelCustom::MOUSE_PRESS_EVENT,false,Qt::NoModifier,Qt::RightButton);
    ((rviz::RenderPanelCustom*)render_panel_)->setEventFilters(rviz::RenderPanelCustom::MOUSE_RELEASE_EVENT,false,Qt::NoModifier,Qt::RightButton);
    ((rviz::RenderPanelCustom*)render_panel_)->setEventFilters(rviz::RenderPanelCustom::MOUSE_MOVE_EVENT,false,Qt::NoModifier,Qt::RightButton);
    //set_goal_tool_->getPropertyContainer()->subProp( "Topic" )->setValue( "/goal_pose_walk" );
    manager_->getToolManager()->setCurrentTool( set_walk_goal_tool_ );
    setting_pose_ = true;
}

void OrthoView::defineStepPosePressed()
{
    //ROS_ERROR("vector pressed in map");
    ((rviz::RenderPanelCustom*)render_panel_)->setEventFilters(rviz::RenderPanelCustom::MOUSE_PRESS_EVENT,false,Qt::NoModifier,Qt::RightButton);
    ((rviz::RenderPanelCustom*)render_panel_)->setEventFilters(rviz::RenderPanelCustom::MOUSE_RELEASE_EVENT,false,Qt::NoModifier,Qt::RightButton);
    ((rviz::RenderPanelCustom*)render_panel_)->setEventFilters(rviz::RenderPanelCustom::MOUSE_MOVE_EVENT,false,Qt::NoModifier,Qt::RightButton);
    //set_goal_tool_->getPropertyContainer()->subProp( "Topic" )->setValue( "/goal_pose_step" );
    manager_->getToolManager()->setCurrentTool( set_step_goal_tool_ );
    setting_pose_ = true;
}


void OrthoView::processGoalPose(const geometry_msgs::PoseStamped::ConstPtr &pose)
{
    //ROS_ERROR("goal processed in map");
    ((rviz::RenderPanelCustom*)render_panel_)->setEventFilters(rviz::RenderPanelCustom::MOUSE_PRESS_EVENT,false,Qt::NoModifier,Qt::LeftButton | Qt::RightButton);
    ((rviz::RenderPanelCustom*)render_panel_)->setEventFilters(rviz::RenderPanelCustom::MOUSE_RELEASE_EVENT,false,Qt::NoModifier,Qt::LeftButton | Qt::RightButton);
    ((rviz::RenderPanelCustom*)render_panel_)->setEventFilters(rviz::RenderPanelCustom::MOUSE_MOVE_EVENT,false,Qt::NoModifier,Qt::LeftButton | Qt::RightButton);
    manager_->getToolManager()->setCurrentTool( move_camera_tool_ );
    setting_pose_ = false;
}

void OrthoView::keyPressEvent( QKeyEvent* event )
{
    // block events and change to camera tool
    ((rviz::RenderPanelCustom*)render_panel_)->setEventFilters(rviz::RenderPanelCustom::MOUSE_PRESS_EVENT,false,Qt::NoModifier,Qt::LeftButton | Qt::RightButton);
    ((rviz::RenderPanelCustom*)render_panel_)->setEventFilters(rviz::RenderPanelCustom::MOUSE_RELEASE_EVENT,false,Qt::NoModifier,Qt::LeftButton | Qt::RightButton);
    ((rviz::RenderPanelCustom*)render_panel_)->setEventFilters(rviz::RenderPanelCustom::MOUSE_MOVE_EVENT,false,Qt::NoModifier,Qt::LeftButton | Qt::RightButton);
    manager_->getToolManager()->setCurrentTool( move_camera_tool_ );
    setting_pose_ = false;
}
}

