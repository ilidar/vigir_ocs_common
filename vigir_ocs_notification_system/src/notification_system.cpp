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
#include "notification_system.h"

#include <boost/thread.hpp>
#include <boost/date_time.hpp>

// Allocating and initializing GlobalClass's
// static data member.  The pointer is being
// allocated - not the object inself
NotificationSystem* NotificationSystem::instance = 0;

NotificationSystem::NotificationSystem()
{    
    //create publisher for notifications
    notification_pub_ = nh.advertise<vigir_ocs_msgs::OCSOverlayText>("/flor/ocs/overlay_text",5,false);    
}

/** This function is called to create an instance of the class.
    Calling the constructor publicly is not allowed. The constructor
    is private and is only called by this Instance function.
*/
NotificationSystem* NotificationSystem::Instance()
{
   if (!instance)   // Only allow one instance of class to be generated.
       instance = new NotificationSystem();

   return instance;
}

//publishes to show white text in top center
void NotificationSystem::notifyPassive(std::string text)
{
    vigir_ocs_msgs::OCSOverlayText msg;
    msg.text = text;
    //messages probably shouldn't be multiple lines?
    msg.height = 18;
    msg.width = text.length() * 8;
    msg.font = "DejaVu Sans Mono";
    msg.line_width = 2;
    msg.text_size = 10;
    msg.fg_color.r = .8f;
    msg.fg_color.g = .8f;
    msg.fg_color.b = .8f;
    msg.fg_color.a = .9f;
    msg.bg_color.a = .5f;
    msg.action = vigir_ocs_msgs::OCSOverlayText::ADD;
    msg.fadeIn = 1.0f;
    //msg.fadeOut = 1.0f;
    msg.upTime = 2.0f;
    msg.row = vigir_ocs_msgs::OCSOverlayText::TOPROW;
    msg.column = vigir_ocs_msgs::OCSOverlayText::CENTERCOLUMN;
    notification_pub_.publish(msg);

}

//publishes to show yellow text in center
void NotificationSystem::notifyWarning(std::string text)
{
    vigir_ocs_msgs::OCSOverlayText msg;
    msg.text = text;
    //messages probably shouldn't be multiple lines?
    msg.height = 18;
    msg.width = text.length() * 8;
    msg.font = "DejaVu Sans Mono";
    msg.line_width = 2;
    msg.text_size = 10;
    msg.fg_color.r = 1.0f;
    msg.fg_color.g = 1.0f;
    msg.fg_color.b = 0.0f;
    msg.fg_color.a = .9f;
    msg.bg_color.a = .5f;
    msg.action = vigir_ocs_msgs::OCSOverlayText::ADD;
    msg.fadeIn = 1.0f;
    msg.fadeOut = 1.0f;
    msg.upTime = 5.0f;
    msg.row = vigir_ocs_msgs::OCSOverlayText::CENTERROW;
    msg.column = vigir_ocs_msgs::OCSOverlayText::CENTERCOLUMN;
    notification_pub_.publish(msg);

    boost::thread thread = boost::thread(&NotificationSystem::sendDelayedNotification, this, 5, text);
}

//publishes to show red text in center
void NotificationSystem::notifyError(std::string text)
{
    vigir_ocs_msgs::OCSOverlayText msg;
    msg.text = text;
    //messages probably shouldn't be multiple lines?
    msg.height = 18;
    msg.width = text.length() * 8;
    msg.font = "DejaVu Sans Mono";
    msg.line_width = 2;
    msg.text_size = 10;
    msg.fg_color.r = 1.0f;
    msg.fg_color.g = 0.0f;
    msg.fg_color.b = 0.0f;
    msg.fg_color.a = .9f;
    msg.bg_color.a = .5f;
    msg.action = vigir_ocs_msgs::OCSOverlayText::ADD;
    msg.fadeIn = 1.0f;
    msg.fadeOut = 1.0f;
    msg.upTime = 5.0f;
    msg.row = vigir_ocs_msgs::OCSOverlayText::CENTERROW;
    msg.column = vigir_ocs_msgs::OCSOverlayText::CENTERCOLUMN;
    notification_pub_.publish(msg);

    boost::thread thread = boost::thread(&NotificationSystem::sendDelayedNotification, this, 5, text);
}

//publishes a custom message to have a notification appear and fade out in 3d view
void NotificationSystem::notifyCustom(std::string text, int row, int column, float r, float g, float b)
{
    vigir_ocs_msgs::OCSOverlayText msg;
    msg.text = text;
    //messages probably shouldn't be multiple lines?
    msg.height = 18;
    msg.width = text.length() * 8;
    msg.font = "DejaVu Sans Mono";
    msg.line_width = 2;
    msg.text_size = 10;
    msg.fg_color.r = r;
    msg.fg_color.g = g;
    msg.fg_color.b = b;
    msg.fg_color.a = .9f;
    msg.bg_color.a = .5f;
    msg.action = vigir_ocs_msgs::OCSOverlayText::ADD;
    msg.fadeIn = 1.0f;
    msg.fadeOut = 1.0f;
    msg.upTime = 2.0f;
    msg.row = row;
    msg.column = column;
    notification_pub_.publish(msg);
}

void NotificationSystem::sendDelayedNotification(int seconds, std::string text)
{
    boost::posix_time::seconds wait_time(seconds);

    boost::this_thread::sleep(wait_time);

    notifyPassive(text);
}


