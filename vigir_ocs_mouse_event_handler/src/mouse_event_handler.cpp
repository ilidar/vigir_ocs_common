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
#include "mouse_event_handler.h"

#include <iostream>

namespace vigir_ocs
{

MouseEventHandler::MouseEventHandler( QObject* parent ) 
    : QObject( parent )
{
}

MouseEventHandler::~MouseEventHandler()
{
}

void MouseEventHandler::mousePressEvent( QMouseEvent* event ) 
{ 
    if( event->button() == Qt::LeftButton )
    {
        Qt::KeyboardModifiers keyMod = event->modifiers();
        bool shift = keyMod.testFlag(Qt::ShiftModifier);
        bool ctrl = keyMod.testFlag(Qt::ControlModifier);

        if( shift && !ctrl ) // as long as shift is pressed
        {
            Q_EMIT mouseLeftButtonShift( true, event->x(), event->y() );
        }
        if( ctrl && !shift ) // if only ctrl is pressed
        {
            Q_EMIT mouseLeftButtonCtrl( true, event->x(), event->y() );
        }

        if( !ctrl && !shift )
        {
            Q_EMIT mouseLeftButton( true, event->x(), event->y() );
        }
    }
    else if( event->button() == Qt::RightButton )
    {
        Q_EMIT mouseRightButton( true, event->x(), event->y() );
    }
}

void MouseEventHandler::mouseReleaseEvent( QMouseEvent* event )
{
    //std::cout << "mouse release event: " << event->button() << " " << event->modifiers() << std::endl;
    if( event->button() == Qt::LeftButton )
    {
        //std::cout << "left button" << std::endl;
        if( event->modifiers() & Qt::ShiftModifier ) // as long as shift is pressed
        {
            //std::cout << "shift" << std::endl;
            // need to emit selection signal here
            Q_EMIT mouseLeftButtonShift( false, event->x(), event->y() );
        }
        else if( event->modifiers() == Qt::NoModifier )
        {
            Q_EMIT mouseLeftButton( false, event->x(), event->y() );
        }
    }
}

void MouseEventHandler::mouseDoubleClick(QMouseEvent * event)
{
    //should only handle left double click
    if( event->button() == Qt::LeftButton )
    {
        Q_EMIT signalMouseLeftDoubleClick(event->x(),event->y());
    }
}

} // namespace rviz
