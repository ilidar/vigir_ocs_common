/*********************************************************************
 * Software License Agreement (BSD License)
 *
 *  Copyright (c) 2013-2015, Team ViGIR ( TORC Robotics LLC, TU Darmstadt, Virginia Tech, Oregon State University, Cornell University, and Leibniz University Hanover )
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above
 *     copyright notice, this list of conditions and the following
 *     disclaimer in the documentation and/or other materials provided
 *     with the distribution.
 *   * Neither the name of Team ViGIR, TORC Robotics, nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 *  FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 *  COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 *  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 *  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 *  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 *  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 *  LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 *  ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *  POSSIBILITY OF SUCH DAMAGE.
 *********************************************************************/
//@TODO_ADD_AUTHOR_INFO
#include "perspective_view_widget.h"
#include "ui_perspective_view_widget.h"
#include "ui/template_loader_widget.h"

PerspectiveViewWidget::PerspectiveViewWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PerspectiveViewWidget)
{
    ui->setupUi(this);

    Q_FOREACH( QDoubleSpinBox * sp, findChildren<QDoubleSpinBox*>() ) {
        sp->installEventFilter( this );
        sp->setFocusPolicy( Qt::StrongFocus );
    }
    Q_FOREACH( QSpinBox * sp, findChildren<QSpinBox*>() ) {
        sp->installEventFilter( this );
        sp->setFocusPolicy( Qt::StrongFocus );
    }
    Q_FOREACH( QComboBox * sp, findChildren<QComboBox*>() ) {
        sp->installEventFilter( this );
        sp->setFocusPolicy( Qt::StrongFocus );
    }
    Q_FOREACH( QSlider * sp, findChildren<QSlider*>() ) {
        sp->installEventFilter( this );
        sp->setFocusPolicy( Qt::StrongFocus );
    }

    ui->octomap_2->hide();
    ui->lidar_point_cloud_2->hide();
    ui->stereo_point_cloud_2->hide();
    ui->laser_scan_2->hide();

    // connect UI to perspective functions
    QObject::connect(ui->camera_tool, SIGNAL(toggled(bool)), ui->perspective_view_, SLOT(cameraToggled(bool)));
    QObject::connect(ui->footstep_planning, SIGNAL(toggled(bool)), ui->perspective_view_, SLOT(footstepPlanningToggled(bool)));
    QObject::connect(ui->footstep_pose, SIGNAL(pressed()), ui->perspective_view_, SLOT(vectorPressed()));
    QObject::connect(ui->grasp_model, SIGNAL(toggled(bool)), ui->perspective_view_, SLOT(graspModelToggled(bool)));
    QObject::connect(ui->grid_map, SIGNAL(toggled(bool)), ui->perspective_view_, SLOT(gridMapToggled(bool)));
    QObject::connect(ui->insert_waypoint, SIGNAL(pressed()), ui->perspective_view_, SLOT(insertWaypoint()));
    QObject::connect(ui->laser_scan_2, SIGNAL(toggled(bool)), ui->perspective_view_, SLOT(laserScanToggled(bool)));
    QObject::connect(ui->lidar_point_cloud_2, SIGNAL(toggled(bool)), ui->perspective_view_, SLOT(lidarPointCloudToggled(bool)));
    QObject::connect(ui->octomap_2, SIGNAL(toggled(bool)), ui->perspective_view_, SLOT(markerArrayToggled(bool)));
    QObject::connect(ui->point_cloud_request, SIGNAL(toggled(bool)), ui->perspective_view_, SLOT(requestedPointCloudToggled(bool)));
    QObject::connect(ui->request_point_cloud_, SIGNAL(clicked()), ui->perspective_view_, SLOT(publishPointCloudWorldRequest()));
    QObject::connect(ui->reset_map, SIGNAL(clicked()), ui->perspective_view_, SLOT(clearMapRequests()));
    QObject::connect(ui->reset_point_cloud, SIGNAL(clicked()), ui->perspective_view_, SLOT(clearPointCloudRequests()));
    QObject::connect(ui->robot_model_2, SIGNAL(toggled(bool)), ui->perspective_view_, SLOT(robotModelToggled(bool)));
    QObject::connect(ui->simulation_robot, SIGNAL(toggled(bool)), ui->perspective_view_, SLOT(simulationRobotToggled(bool)));
    QObject::connect(ui->stereo_point_cloud_2, SIGNAL(toggled(bool)), ui->perspective_view_, SLOT(stereoPointCloudToggled(bool)));
    QObject::connect(ui->template_tool, SIGNAL(toggled(bool)), ui->perspective_view_, SLOT(markerTemplateToggled(bool)));
    QObject::connect(ui->template_widget, SIGNAL(insertTemplate(QString)), ui->perspective_view_, SLOT(insertTemplate(QString)));
    QObject::connect(ui->template_widget, SIGNAL(templatePathChanged(QString)), ui->perspective_view_, SLOT(templatePathChanged(QString)));
    QObject::connect(ui->templates, SIGNAL(toggled(bool)), ui->perspective_view_, SLOT(templatesToggled(bool)));
    QObject::connect(ui->widget_tool, SIGNAL(toggled(bool)), ui->perspective_view_, SLOT(markerRobotToggled(bool)));

}

PerspectiveViewWidget::~PerspectiveViewWidget()
{
    delete ui;
}

bool PerspectiveViewWidget::eventFilter( QObject * o, QEvent * e )
{
    if ( e->type() == QEvent::Wheel &&
         (qobject_cast<QAbstractSpinBox*>( o ) || qobject_cast<QAbstractSlider*>( o ) || qobject_cast<QComboBox*>( o )))
    {
        e->ignore();
        return true;
    }
    return QWidget::eventFilter( o, e );
}
