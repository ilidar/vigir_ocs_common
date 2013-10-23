#include <ros/package.h>

#include "image_video_manager_widget.h"
#include "ui_image_video_manager_widget.h"
#include "stdio.h"
#include <iostream>
#include <QPainter>
#include <QtGui>
#include <QSignalMapper>


#include <cv_bridge/cv_bridge.h>
#include <sensor_msgs/image_encodings.h>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include<QTreeWidget>
#include <QTreeWidgetItem>



ImageVideoManagerWidget::ImageVideoManagerWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ImageVideoManagerWidget)
{
    ui->setupUi(this);
    feed_rate_l=feed_rate_r=feed_rate_lhl=feed_rate_lhr=feed_rate_rhl=feed_rate_rhr= -1;
    feed_rate_prev_l=feed_rate_prev_r=feed_rate_prev_lhl=feed_rate_prev_lhr=feed_rate_prev_rhl=feed_rate_prev_rhr = -1;
    ui->timeslider->setMinimum(0);
    ui->timeslider->setMaximum(0);
    imagecount_l=imagecount_r=imagecount_lhl=imagecount_lhr=imagecount_rhl=imagecount_rhr=0;
    videocount_l=videocount_lhl=videocount_lhr=videocount_r=videocount_rhr=videocount_rhl=0;
    interval_count_l=interval_count_lhl=interval_count_lhr=interval_count_r=interval_count_rhr=interval_count_rhl=0;
    video_start_time_l=video_start_time_r=video_start_time_lhr=video_start_time_lhl=video_start_time_rhr=video_start_time_rhl=0;
    subseq_video_time_l=subseq_video_time_r=subseq_video_time_lhr=subseq_video_time_lhl=subseq_video_time_rhr=subseq_video_time_rhl=0;

    ui->treeWidget->setColumnWidth(0,120);

    // initialize publishers for communication with nodelet
    image_list_request_pub_ = nh_.advertise<std_msgs::Bool>(   "/flor/ocs/image_history/list_request", 1, true );
    image_selected_pub_     = nh_.advertise<std_msgs::UInt64>( "/flor/ocs/image_history/select_image", 1, false );

    image_added_sub_    = nh_.subscribe<flor_ocs_msgs::OCSImageAdd>(  "/flor/ocs/image_history/add",  5, &ImageVideoManagerWidget::processImageAdd,  this );
    image_list_sub_     = nh_.subscribe<flor_ocs_msgs::OCSImageList>( "/flor/ocs/image_history/list", 100, &ImageVideoManagerWidget::processImageList, this );

    // uncomment later
     image_selected_sub_ = nh_.subscribe<sensor_msgs::Image>( "/flor/ocs/history/image_raw", 5, &ImageVideoManagerWidget::processSelectedImage, this );

    // sunscribers for video stream

    img_req_sub_full_l_ = nh_.subscribe<flor_perception_msgs::DownSampledImageRequest>( "/l_image_full/image_request", 1, &ImageVideoManagerWidget::processvideoimage_l, this );
    img_req_sub_full_r_ = nh_.subscribe<flor_perception_msgs::DownSampledImageRequest>( "/r_image_full/image_request", 1, &ImageVideoManagerWidget::processvideoimage_r, this );
    img_req_sub_full_lhl_ = nh_.subscribe<flor_perception_msgs::DownSampledImageRequest>( "/lhl_image_full/image_request", 1, &ImageVideoManagerWidget::processvideoimage_lhl, this );
    img_req_sub_full_lhr_ = nh_.subscribe<flor_perception_msgs::DownSampledImageRequest>( "/lhr_image_full/image_request", 1, &ImageVideoManagerWidget::processvideoimage_lhr, this );
    img_req_sub_full_rhl_ = nh_.subscribe<flor_perception_msgs::DownSampledImageRequest>( "/rhl_image_full/image_request", 1, &ImageVideoManagerWidget::processvideoimage_rhl, this );
    img_req_sub_full_rhr_ = nh_.subscribe<flor_perception_msgs::DownSampledImageRequest>( "/rhr_image_full/image_request", 1, &ImageVideoManagerWidget::processvideoimage_rhr, this );

    img_req_sub_crop_l_ = nh_.subscribe<flor_perception_msgs::DownSampledImageRequest>( "/l_image_cropped/image_request", 1, &ImageVideoManagerWidget::processvideoimage_l, this );
    img_req_sub_crop_r_ = nh_.subscribe<flor_perception_msgs::DownSampledImageRequest>( "/r_image_cropped/image_request", 1, &ImageVideoManagerWidget::processvideoimage_r, this );
    img_req_sub_crop_lhl_ = nh_.subscribe<flor_perception_msgs::DownSampledImageRequest>( "/lhl_image_cropped/image_request", 1, &ImageVideoManagerWidget::processvideoimage_lhl, this );
    img_req_sub_crop_lhr_ = nh_.subscribe<flor_perception_msgs::DownSampledImageRequest>( "/lhr_image_cropped/image_request", 1, &ImageVideoManagerWidget::processvideoimage_lhr, this );
    img_req_sub_crop_rhl_ = nh_.subscribe<flor_perception_msgs::DownSampledImageRequest>( "/rhl_image_cropped/image_request", 1, &ImageVideoManagerWidget::processvideoimage_rhl, this );
    img_req_sub_crop_rhr_ = nh_.subscribe<flor_perception_msgs::DownSampledImageRequest>( "/rhr_image_cropped/image_request", 1, &ImageVideoManagerWidget::processvideoimage_rhr, this );
    //connect(ui->treeWidget, SIGNAL(cellClicked(int,int)), this, SLOT(editSlot(int, int)));
    std_msgs::Bool list_request;
    list_request.data = true;
    image_list_request_pub_.publish(list_request);

    timer.start(33, this);

}

ImageVideoManagerWidget::~ImageVideoManagerWidget()
{
    delete ui;
}

void ImageVideoManagerWidget::timerEvent(QTimerEvent *event)
{
    //Spin at beginning of Qt timer callback, so current ROS time is retrieved
    ros::spinOnce();
}

QImage Mat2QImage(const cv::Mat3b &src)
{
    QImage dest(src.cols, src.rows, QImage::Format_ARGB32);
    for (int y = 0; y < src.rows; ++y)
    {
        const cv::Vec3b *srcrow = src[y];
        QRgb *destrow = (QRgb*)dest.scanLine(y);
        for (int x = 0; x < src.cols; ++x)
        {
            destrow[x] = qRgba(srcrow[x][2], srcrow[x][1], srcrow[x][0], 255);
        }
    }
    return dest;
}

void ImageVideoManagerWidget::addImage(const unsigned long& id, const std::string& topic, const sensor_msgs::Image& image, const sensor_msgs::CameraInfo& camera_info)
{
    /*int row = 0;
    ui->treeWidget->add;
    ui->treeWidget->setRowHeight(row,100);*/

    // add info about images to the table

    QTreeWidgetItem * item;

    // image
    item = new QTreeWidgetItem();

    //ROS_ERROR("Encoding: %s", image.encoding.c_str());

    double aspect_ratio = (double)image.width/(double)image.height;
    //ROS_ERROR("Size: %dx%d aspect %f", image.width, image.height, aspect_ratio);

    cv_bridge::CvImagePtr cv_ptr;
    try
    {
        cv_ptr = cv_bridge::toCvCopy(image, sensor_msgs::image_encodings::BGR8);
    }
    catch (cv_bridge::Exception& e)
    {
       // ROS_ERROR("cv_bridge exception: %s", e.what());
        return;
    }

    cv::Size2i img_size;
    if(aspect_ratio > 1)
    {
        //img_size.width = 100.0f;
        //img_size.height = 100.0f/aspect_ratio;
        img_size.width = 50.0f;
        img_size.height = 50.0f/aspect_ratio;
    }
    else
    {
        img_size.width = 50.0f/aspect_ratio;
        img_size.height = 50.0f;
    }
    cv::resize(cv_ptr->image, cv_ptr->image, img_size, 0, 0, cv::INTER_NEAREST);
    QImage tmp = Mat2QImage(cv_ptr->image);
    QPixmap pixmap = QPixmap::fromImage(tmp);
    item->setData(0,Qt::DecorationRole, pixmap);

    item->setToolTip(0,QString::number(id));
    //ui->treeWidget->setItem(row,0,item);

   // ROS_ERROR("Added %ld to the table",id);

    // stamp
    //item = new QTreeWidgetItem(timeFromMsg(image.header.stamp));
    item->setText(1,timeFromMsg(image.header.stamp));
    // source
    //item = new QTreeWidgeexttItem(QString(topic.c_str()));
    item->setText(2,QString(topic.c_str()));
    // width
   // item = new QTreeWidgetItem(QString::number(image.width));
    item->setText(3,QString::number(image.width));
    // height
   // item = new QTreeWidgetItem(QString::number(image.height));
    item->setText(4,QString::number(image.height));
    ui->treeWidget->addTopLevelItem(item);
}
void ImageVideoManagerWidget::thumbnail(const sensor_msgs::Image& image,QTreeWidgetItem *item)
{


    //ROS_ERROR("Encoding: %s", image.encoding.c_str());

    double aspect_ratio = (double)image.width/(double)image.height;
   // ROS_ERROR("Size: %dx%d aspect %f", image.width, image.height, aspect_ratio);

    cv_bridge::CvImagePtr cv_ptr;
    try
    {
        cv_ptr = cv_bridge::toCvCopy(image, sensor_msgs::image_encodings::BGR8);
    }
    catch (cv_bridge::Exception& e)
    {
       // ROS_ERROR("cv_bridge exception: %s", e.what());
        return;
    }

    cv::Size2i img_size;
    if(aspect_ratio > 1)
    {
        img_size.width = 50.0f;
        img_size.height = 50.0f/aspect_ratio;
    }
    else
    {
        img_size.width = 50.0f/aspect_ratio;
        img_size.height = 50.0f;
    }
    cv::resize(cv_ptr->image, cv_ptr->image, img_size, 0, 0, cv::INTER_NEAREST);
    QImage tmp = Mat2QImage(cv_ptr->image);
    QPixmap pixmap = QPixmap::fromImage(tmp);
    item->setData(0,Qt::DecorationRole, pixmap);


}
void ImageVideoManagerWidget::addImageChild(QTreeWidgetItem *parent, const unsigned long& id, const std::string& topic, const sensor_msgs::Image& image, const sensor_msgs::CameraInfo& camera_info,int flag)
// flag to distinguish between image and video
{
    /*int row = 0;
    ui->treeWidget->add;
    ui->treeWidget->setRowHeight(row,100);*/

    // add info about images to the table

    QTreeWidgetItem * item;

    // image
    item = new QTreeWidgetItem();

    //ROS_ERROR("Encoding: %s", image.encoding.c_str());

    double aspect_ratio = (double)image.width/(double)image.height;
   // ROS_ERROR("Size: %dx%d aspect %f", image.width, image.height, aspect_ratio);

    cv_bridge::CvImagePtr cv_ptr;
    try
    {
        cv_ptr = cv_bridge::toCvCopy(image, sensor_msgs::image_encodings::BGR8);
    }
    catch (cv_bridge::Exception& e)
    {
       // ROS_ERROR("cv_bridge exception: %s", e.what());
        return;
    }

    cv::Size2i img_size;
    if(aspect_ratio > 1)
    {
        img_size.width = 50.0f;
        img_size.height = 50.0f/aspect_ratio;
    }
    else
    {
        img_size.width = 50.0f/aspect_ratio;
        img_size.height = 50.0f;
    }
    cv::resize(cv_ptr->image, cv_ptr->image, img_size, 0, 0, cv::INTER_NEAREST);
    QImage tmp = Mat2QImage(cv_ptr->image);
    QPixmap pixmap = QPixmap::fromImage(tmp);
    item->setData(0,Qt::DecorationRole, pixmap);

    item->setToolTip(0,QString::number(id));
    //ui->treeWidget->setItem(row,0,item);

    //ROS_ERROR("Added %ld to the table",id);

    // stamp
    //item = new QTreeWidgetItem(timeFromMsg(image.header.stamp));
    item->setText(1,timeFromMsg(image.header.stamp));
    item->setToolTip(1,QString::number((int)image.header.stamp.toSec()));
    // source
    //item = new QTreeWidgeexttItem(QString(topic.c_str()));
    item->setText(2,QString(topic.c_str()));
    // width
   // item = new QTreeWidgetItem(QString::number(image.width));
    item->setText(3,QString::number(image.width));
    // height
   // item = new QTreeWidgetItem(QString::number(image.height));
    item->setText(4,QString::number(image.height));
    if(flag==0)
    {
       parent=item;
       ui->treeWidget->insertTopLevelItem(0,item);
    }
    else
    {
    parent->setText(1,timeFromMsg(image.header.stamp));
    thumbnail(image,parent);

    //parent->setText(2,QString(topic.c_str()));
    //parent->parent()->setText(1,timeFromMsg(image.header.stamp));
    parent->addChild(item);
    while(parent)
      {

        parent->setText(1,timeFromMsg(image.header.stamp));
        parent->setText(2,QString(topic.c_str()));
        thumbnail(image,parent);
        parent= parent->parent();
      }
    }
}

QTreeWidgetItem* ImageVideoManagerWidget::addvideoitem(int videocount,const sensor_msgs::Image& image)

{
    QTreeWidgetItem * item;
    item = new QTreeWidgetItem();
    //item->setText(0,"Video"+QString::number(videocount));
    item->setText(1,timeFromMsg(image.header.stamp));
    item->setToolTip(1,QString::number((int)image.header.stamp.toSec()));
    ui->treeWidget->insertTopLevelItem(0,item);
    return item;
}
QTreeWidgetItem* ImageVideoManagerWidget::add_time_child(QTreeWidgetItem *pitem,int count_n)

{
    QTreeWidgetItem * iteml;
    iteml = new QTreeWidgetItem();
    //iteml->setText(0,QString::number(count_n));
   // item->setText(1,timeFromMsg(image.header.stamp));
   // item->setToolTip(1,QString::number((int)image.header.stamp.toSec()));
    pitem->addChild(iteml);
    return iteml;
}
/*QTreeWidgetItem* ImageVideoManagerWidget::addimageitem(int imagecount,const sensor_msgs::Image& image)
// adding image events on getfeed()
{
    ROS_ERROR("in add image");
    QTreeWidgetItem * item;
   // item = new QTreeWidgetItem();
    //thumbnail(image,item);
    //item->setText(0,"Image"+QString::number(imagecount));
    //item->setText(1,timeFromMsg(image.header.stamp));
   // item->setToolTip(1,QString::number((int)image.header.stamp.toSec()));
    ui->treeWidget->insertTopLevelItem(0,item);
    return item;
}*/
void ImageVideoManagerWidget::processImageAdd(const flor_ocs_msgs::OCSImageAdd::ConstPtr &msg)
{

    //ROS_ERROR(msg->topic.c_str());

    if(QString(msg->topic.c_str())=="/l_image_full" || QString(msg->topic.c_str())=="/l_image_cropped")
        imageaddfunction_l(msg);
   if(QString(msg->topic.c_str())=="/r_image_full" || QString(msg->topic.c_str())=="/r_image_cropped")
        imageaddfunction_r(msg);
    if(QString(msg->topic.c_str())=="/lhl_image_full" || QString(msg->topic.c_str())=="/lhl_image_cropped")
        imageaddfunction_lhl(msg);
    if(QString(msg->topic.c_str())=="/lhr_image_full" || QString(msg->topic.c_str())=="/lhr_image_cropped")
        imageaddfunction_lhr(msg);
    if(QString(msg->topic.c_str())=="/rhl_image_full" || QString(msg->topic.c_str())=="/rhl_image_cropped")
        imageaddfunction_rhl(msg);
    if(QString(msg->topic.c_str())=="/rhr_image_full" || QString(msg->topic.c_str())=="/rhr_image_cropped")
        imageaddfunction_rhr(msg);


 }
void ImageVideoManagerWidget:: imageaddfunction_l(const flor_ocs_msgs::OCSImageAdd::ConstPtr &msg)
{

    if(feed_rate_l==0.0f)
      {
        imagecount_l++;
      //  item_l = addimageitem(imagecount_l,msg->image);
        addImageChild(item_l,msg->id,msg->topic,msg->image,msg->camera_info,0);
        ui->timeslider->setMaximum((msg->image).header.stamp.toSec());// in seconds. nano seconds not counted
      }

    else
        if(feed_rate_l>0.0f)
    {   ROS_ERROR("in l image");
        subseq_video_time_l=(int)msg->image.header.stamp.toSec();
        //subseq_video_time_nano= ((msg->image).header.stamp.toSec() - (int)(msg->image).header.stamp.toSec())*1000;
        qDebug()<<subseq_video_time_l;
        if(feed_rate_l!=feed_rate_prev_l)
        {

            videocount_l++;
            parentitem_l=addvideoitem(videocount_l,msg->image);
            interval_count_l=1;
            video_start_time_l=(int)msg->image.header.stamp.toSec();
            timeitem_l=add_time_child(parentitem_l,5);
            childtimeitem_l = add_time_child(timeitem_l,1);
            childtimeitem_start_time_l = (int)msg->image.header.stamp.toSec();
            //video_start_time_nano=((msg->image).header.stamp.toSec() - (int)(msg->image).header.stamp.toSec())*1000;

        }

        if((subseq_video_time_l-video_start_time_l)>300)
     {
         interval_count_l++;
         timeitem_l=add_time_child(parentitem_l,5);
         video_start_time_l=(int)msg->image.header.stamp.toSec();
         childtimeitem_l = add_time_child(timeitem_l,1);
         childtimeitem_start_time_l = (int)msg->image.header.stamp.toSec();

     }
        if((subseq_video_time_l-childtimeitem_start_time_l)>60)
     {

            childtimeitem_l = add_time_child(timeitem_l,1);
            childtimeitem_start_time_l = (int)msg->image.header.stamp.toSec();

     }
     addImageChild(childtimeitem_l,msg->id,msg->topic,msg->image,msg->camera_info,1);
     ui->timeslider->setMaximum((msg->image).header.stamp.toSec());

     }
       feed_rate_prev_l = feed_rate_l;

}

void ImageVideoManagerWidget:: imageaddfunction_r(const flor_ocs_msgs::OCSImageAdd::ConstPtr &msg)
{

    if(feed_rate_r==0.0f)
      {
        ROS_ERROR("in r image");
        imagecount_r++;
        //item_r = addimageitem(imagecount_r,msg->image);
        addImageChild(item_r,msg->id,msg->topic,msg->image,msg->camera_info,0);
        ui->timeslider->setMaximum((msg->image).header.stamp.toSec());// in seconds. nano seconds not counted
      }

    else
        if(feed_rate_r>0.0f)
    {
        subseq_video_time_r=(int)msg->image.header.stamp.toSec();
        //subseq_video_time_nano= ((msg->image).header.stamp.toSec() - (int)(msg->image).header.stamp.toSec())*1000;
        qDebug()<<subseq_video_time_r;
        if(feed_rate_r!=feed_rate_prev_r)
        {
            videocount_r++;
            parentitem_r=addvideoitem(videocount_r,msg->image);
            interval_count_r=1;
            video_start_time_r=(int)msg->image.header.stamp.toSec();
            timeitem_r=add_time_child(parentitem_r,5);
            childtimeitem_r = add_time_child(timeitem_r,1);
            childtimeitem_start_time_r = (int)msg->image.header.stamp.toSec();
            //video_start_time_nano=((msg->image).header.stamp.toSec() - (int)(msg->image).header.stamp.toSec())*1000;

        }

        if((subseq_video_time_r-video_start_time_r)>300)
     {
         interval_count_r++;
         timeitem_r=add_time_child(parentitem_r,5);
         video_start_time_r=(int)msg->image.header.stamp.toSec();
         childtimeitem_r = add_time_child(timeitem_r,1);
         childtimeitem_start_time_r = (int)msg->image.header.stamp.toSec();

     }
        if((subseq_video_time_r-childtimeitem_start_time_r)>60)
     {

            childtimeitem_r = add_time_child(timeitem_r,1);
            childtimeitem_start_time_r = (int)msg->image.header.stamp.toSec();

     }
     addImageChild(childtimeitem_r,msg->id,msg->topic,msg->image,msg->camera_info,1);
     ui->timeslider->setMaximum((msg->image).header.stamp.toSec());

     }
       feed_rate_prev_r = feed_rate_r;

}
void ImageVideoManagerWidget:: imageaddfunction_lhl(const flor_ocs_msgs::OCSImageAdd::ConstPtr &msg)
{
    ROS_ERROR("in lhl image");
    if(feed_rate_lhl==0.0f)
      {
        imagecount_lhl++;
        //item_lhl = addimageitem(imagecount_lhl,msg->image);
        addImageChild(item_lhl,msg->id,msg->topic,msg->image,msg->camera_info,0);
        ui->timeslider->setMaximum((msg->image).header.stamp.toSec());// in seconds. nano seconds not counted
      }

    else
        if(feed_rate_lhl>0.0f)
    {
        subseq_video_time_lhl=(int)msg->image.header.stamp.toSec();
        //subseq_video_time_nano= ((msg->image).header.stamp.toSec() - (int)(msg->image).header.stamp.toSec())*1000;
        qDebug()<<subseq_video_time_lhl;
        if(feed_rate_lhl!=feed_rate_prev_lhl)
        {
            videocount_lhl++;
            parentitem_lhl=addvideoitem(videocount_lhl,msg->image);
            interval_count_lhl=1;
            video_start_time_lhl=(int)msg->image.header.stamp.toSec();
            timeitem_lhl=add_time_child(parentitem_lhl,5);
            childtimeitem_lhl = add_time_child(timeitem_lhl,1);
            childtimeitem_start_time_lhl = (int)msg->image.header.stamp.toSec();
            //video_start_time_nano=((msg->image).header.stamp.toSec() - (int)(msg->image).header.stamp.toSec())*1000;

        }

        if((subseq_video_time_lhl-video_start_time_lhl)>300)
     {
         interval_count_lhl++;
         timeitem_lhl=add_time_child(parentitem_lhl,5);
         video_start_time_lhl=(int)msg->image.header.stamp.toSec();
         childtimeitem_lhl = add_time_child(timeitem_lhl,1);
         childtimeitem_start_time_lhl = (int)msg->image.header.stamp.toSec();

     }
        if((subseq_video_time_lhl-childtimeitem_start_time_lhl)>60)
     {

            childtimeitem_lhl = add_time_child(timeitem_lhl,1);
            childtimeitem_start_time_lhl = (int)msg->image.header.stamp.toSec();

     }
     addImageChild(childtimeitem_lhl,msg->id,msg->topic,msg->image,msg->camera_info,1);
     ui->timeslider->setMaximum((msg->image).header.stamp.toSec());

     }
       feed_rate_prev_lhl = feed_rate_lhl;

}
void ImageVideoManagerWidget:: imageaddfunction_lhr(const flor_ocs_msgs::OCSImageAdd::ConstPtr &msg)
{
    ROS_ERROR("in lhr image");
    if(feed_rate_lhr==0.0f)
      {
        imagecount_lhr++;
       // item_lhr = addimageitem(imagecount_lhr,msg->image);
        addImageChild(item_lhr,msg->id,msg->topic,msg->image,msg->camera_info,0);
        ui->timeslider->setMaximum((msg->image).header.stamp.toSec());// in seconds. nano seconds not counted
      }

    else
        if(feed_rate_lhr>0.0f)
    {
        subseq_video_time_lhr=(int)msg->image.header.stamp.toSec();
        //subseq_video_time_nano= ((msg->image).header.stamp.toSec() - (int)(msg->image).header.stamp.toSec())*1000;
        qDebug()<<subseq_video_time_lhr;
        if(feed_rate_lhr!=feed_rate_prev_lhr)
        {
            videocount_lhr++;
            parentitem_lhr=addvideoitem(videocount_lhr,msg->image);
            interval_count_lhr=1;
            video_start_time_lhr=(int)msg->image.header.stamp.toSec();
            timeitem_lhr=add_time_child(parentitem_lhr,5);
            childtimeitem_lhr = add_time_child(timeitem_lhr,1);
            childtimeitem_start_time_lhr = (int)msg->image.header.stamp.toSec();
            //video_start_time_nano=((msg->image).header.stamp.toSec() - (int)(msg->image).header.stamp.toSec())*1000;

        }

        if((subseq_video_time_lhr-video_start_time_lhr)>300)
     {
         interval_count_lhr++;
         timeitem_lhr=add_time_child(parentitem_lhr,5);
         video_start_time_lhr=(int)msg->image.header.stamp.toSec();
         childtimeitem_lhr = add_time_child(timeitem_lhr,1);
         childtimeitem_start_time_lhr = (int)msg->image.header.stamp.toSec();

     }
        if((subseq_video_time_lhr-childtimeitem_start_time_lhr)>60)
     {

            childtimeitem_lhr = add_time_child(timeitem_lhr,1);
            childtimeitem_start_time_lhr = (int)msg->image.header.stamp.toSec();

     }
     addImageChild(childtimeitem_lhr,msg->id,msg->topic,msg->image,msg->camera_info,1);
     ui->timeslider->setMaximum((msg->image).header.stamp.toSec());

     }
       feed_rate_prev_lhr = feed_rate_lhr;

}
void ImageVideoManagerWidget:: imageaddfunction_rhr(const flor_ocs_msgs::OCSImageAdd::ConstPtr &msg)
{
    ROS_ERROR("in rhr image");
    if(feed_rate_rhr==0.0f)
      {
        imagecount_rhr++;
        //item_rhr = addimageitem(imagecount_rhr,msg->image);
        addImageChild(item_rhr,msg->id,msg->topic,msg->image,msg->camera_info,0);
        ui->timeslider->setMaximum((msg->image).header.stamp.toSec());// in seconds. nano seconds not counted
      }

    else
        if(feed_rate_rhr>0.0f)
    {
        subseq_video_time_rhr=(int)msg->image.header.stamp.toSec();
        //subseq_video_time_nano= ((msg->image).header.stamp.toSec() - (int)(msg->image).header.stamp.toSec())*1000;
        qDebug()<<subseq_video_time_rhr;
        if(feed_rate_rhr!=feed_rate_prev_rhr)
        {
            videocount_rhr++;
            parentitem_rhr=addvideoitem(videocount_rhr,msg->image);
            interval_count_rhr=1;
            video_start_time_rhr=(int)msg->image.header.stamp.toSec();
            timeitem_rhr=add_time_child(parentitem_rhr,5);
            childtimeitem_rhr = add_time_child(timeitem_rhr,1);
            childtimeitem_start_time_rhr = (int)msg->image.header.stamp.toSec();
            //video_start_time_nano=((msg->image).header.stamp.toSec() - (int)(msg->image).header.stamp.toSec())*1000;

        }

        if((subseq_video_time_rhr-video_start_time_rhr)>300)
     {
         interval_count_rhr++;
         timeitem_rhr=add_time_child(parentitem_rhr,5);
         video_start_time_rhr=(int)msg->image.header.stamp.toSec();
         childtimeitem_rhr = add_time_child(timeitem_rhr,1);
         childtimeitem_start_time_rhr = (int)msg->image.header.stamp.toSec();

     }
        if((subseq_video_time_rhr-childtimeitem_start_time_rhr)>60)
     {

            childtimeitem_rhr = add_time_child(timeitem_rhr,1);
            childtimeitem_start_time_rhr = (int)msg->image.header.stamp.toSec();

     }
     addImageChild(childtimeitem_rhr,msg->id,msg->topic,msg->image,msg->camera_info,1);
     ui->timeslider->setMaximum((msg->image).header.stamp.toSec());

     }
       feed_rate_prev_rhr = feed_rate_rhr;

}
void ImageVideoManagerWidget:: imageaddfunction_rhl(const flor_ocs_msgs::OCSImageAdd::ConstPtr &msg)
{
   ROS_ERROR("in rhl image");
    if(feed_rate_rhl==0.0f)
      {
        imagecount_rhl++;
        //item_rhl = addimageitem(imagecount_rhl,msg->image);
        addImageChild(item_rhl,msg->id,msg->topic,msg->image,msg->camera_info,0);
        ui->timeslider->setMaximum((msg->image).header.stamp.toSec());// in seconds. nano seconds not counted
      }

    else
        if(feed_rate_rhl>0.0f)
    {
        subseq_video_time_rhl=(int)msg->image.header.stamp.toSec();
        //subseq_video_time_nano= ((msg->image).header.stamp.toSec() - (int)(msg->image).header.stamp.toSec())*1000;
        qDebug()<<subseq_video_time_rhl;
        if(feed_rate_rhl!=feed_rate_prev_rhl)
        {
            videocount_rhl++;
            parentitem_rhl=addvideoitem(videocount_rhl,msg->image);
            interval_count_rhl=1;
            video_start_time_rhl=(int)msg->image.header.stamp.toSec();
            timeitem_rhl=add_time_child(parentitem_rhl,5);
            childtimeitem_rhl = add_time_child(timeitem_rhl,1);
            childtimeitem_start_time_rhl = (int)msg->image.header.stamp.toSec();
            //video_start_time_nano=((msg->image).header.stamp.toSec() - (int)(msg->image).header.stamp.toSec())*1000;

        }

        if((subseq_video_time_rhl-video_start_time_rhl)>300)
     {
         interval_count_rhl++;
         timeitem_rhl=add_time_child(parentitem_rhl,5);
         video_start_time_rhl=(int)msg->image.header.stamp.toSec();
         childtimeitem_rhl = add_time_child(timeitem_rhl,1);
         childtimeitem_start_time_rhl = (int)msg->image.header.stamp.toSec();

     }
        if((subseq_video_time_rhl-childtimeitem_start_time_rhl)>60)
     {

            childtimeitem_rhl = add_time_child(timeitem_rhl,1);
            childtimeitem_start_time_rhl = (int)msg->image.header.stamp.toSec();

     }
     addImageChild(childtimeitem_rhl,msg->id,msg->topic,msg->image,msg->camera_info,1);
     ui->timeslider->setMaximum((msg->image).header.stamp.toSec());

     }
       feed_rate_prev_rhl = feed_rate_rhl;

}

void ImageVideoManagerWidget::processImageList(const flor_ocs_msgs::OCSImageList::ConstPtr& msg)
{
    // reset table
    //ui->treeWidget->clear();
    ROS_ERROR("in process image list");




}

void ImageVideoManagerWidget::processSelectedImage(const sensor_msgs::Image::ConstPtr &msg)
{
    // image
    ROS_ERROR("Encoding: %s", msg->encoding.c_str());

    double aspect_ratio = (double)msg->width/(double)msg->height;
   // ROS_ERROR("Size: %dx%d aspect %f", msg->width, msg->height, aspect_ratio);

    cv_bridge::CvImagePtr cv_ptr;
    try
    {
        cv_ptr = cv_bridge::toCvCopy(*msg, sensor_msgs::image_encodings::BGR8);
    }
    catch (cv_bridge::Exception& e)
    {
        //ROS_ERROR("cv_bridge exception: %s", e.what());
        return;
    }

    cv::Size2i img_size;
    if(aspect_ratio > 1)
    {
        img_size.width = ui->image_view->width();
        img_size.height = ((float)ui->image_view->width())/aspect_ratio;
    }
    else
    {
        img_size.width = ((float)ui->image_view->height())/aspect_ratio;
        img_size.height = ui->image_view->height();
    }
    cv::resize(cv_ptr->image, cv_ptr->image, img_size, 0, 0, cv::INTER_NEAREST);
    QImage tmp = Mat2QImage(cv_ptr->image);
    QPixmap pixmap = QPixmap::fromImage(tmp);
    ui->image_view->setPixmap(pixmap);
}

QString ImageVideoManagerWidget::timeFromMsg(const ros::Time& stamp)
{
    int sec = stamp.toSec();
    std::stringstream stream;

    stream.str("");
    int day = sec/86400;
    sec -= day * 86400;

    int hour = sec / 3600;
    sec -= hour * 3600;

    int min = sec / 60;
    sec -= min * 60;
    uint32_t nano = (stamp.toSec() - (int)stamp.toSec())*1000;
    //stream << std::setw(2) << std::setfill('0') << day << " ";
    //stream << std::setw(2) << std::setfill('0') << hour << ":";
    stream << std::setw(2) << std::setfill('0') << min << ":";
    stream << std::setw(2) << std::setfill('0') << sec << ".";
    stream << std::setw(3) << std::setfill('0') << nano;
    return QString::fromStdString(stream.str());
}
void ImageVideoManagerWidget::processvideoimage_l (const flor_perception_msgs::DownSampledImageRequest::ConstPtr& msg)
{
    //if(msg->mode==flor_perception_msgs::DownSampledImageRequest::PUBLISH_FREQ)
        feed_rate_l = msg->publish_frequency;
    ROS_ERROR("in process video imagel.feed rate = %f", feed_rate_l);

}
void ImageVideoManagerWidget::processvideoimage_r (const flor_perception_msgs::DownSampledImageRequest::ConstPtr& msg)
{
    //if(msg->mode==flor_perception_msgs::DownSampledImageRequest::PUBLISH_FREQ)
        feed_rate_r = msg->publish_frequency;
    ROS_ERROR("in process video imager.feed rate = %f", feed_rate_r);

}
void ImageVideoManagerWidget::processvideoimage_lhl (const flor_perception_msgs::DownSampledImageRequest::ConstPtr& msg)
{
    //if(msg->mode==flor_perception_msgs::DownSampledImageRequest::PUBLISH_FREQ)
        feed_rate_lhl = msg->publish_frequency;
    ROS_ERROR("in process video imagelhl.feed rate = %f", feed_rate_lhl);

}
void ImageVideoManagerWidget::processvideoimage_lhr (const flor_perception_msgs::DownSampledImageRequest::ConstPtr& msg)
{
    //if(msg->mode==flor_perception_msgs::DownSampledImageRequest::PUBLISH_FREQ)
        feed_rate_lhr = msg->publish_frequency;
    ROS_ERROR("in process video imagelhr.feed rate = %f", feed_rate_lhr);

}
void ImageVideoManagerWidget::processvideoimage_rhl (const flor_perception_msgs::DownSampledImageRequest::ConstPtr& msg)
{
    //if(msg->mode==flor_perception_msgs::DownSampledImageRequest::PUBLISH_FREQ)
        feed_rate_rhl = msg->publish_frequency;
    ROS_ERROR("in process video imagerhl.feed rate = %f", feed_rate_rhl);

}
void ImageVideoManagerWidget::processvideoimage_rhr (const flor_perception_msgs::DownSampledImageRequest::ConstPtr& msg)
{
    //if(msg->mode==flor_perception_msgs::DownSampledImageRequest::PUBLISH_FREQ)
        feed_rate_rhr = msg->publish_frequency;
    ROS_ERROR("in process video imagerhr.feed rate = %f", feed_rate_rhr);

}





void ImageVideoManagerWidget::on_treeWidget_itemClicked(QTreeWidgetItem *item, int column)
{
    if(item->parent()!=0)  // check if item clicked is parent or child
    {
        std_msgs::UInt64 request;
        request.data = item->toolTip(0).toULong();
       // ui->image_view->setText(item->toolTip(0));
        image_selected_pub_.publish(request);


    }
    else
        ui->image_view->setText("No Image Selected!!");

}




void ImageVideoManagerWidget::on_timeslider_valueChanged(int value)
{
    //ui->timelabel->setText("Time(s):"+QString::number(value));
    ui->timelabel->setNum(value);

}
bool ImageVideoManagerWidget::check_item_time(QTreeWidgetItem *item, int time)
{
    qDebug()<<"item time"<<item->toolTip(1);
    qDebug()<<"time label:"<<QString::number(time);
    if(item->toolTip(1).toInt()<=time)
       return true;
    else
       return false;


}
/*
void ImageVideoManagerWidget:: settree_show()
{
    for(int j=0;j<videocount+imagecount;j++)
    {
   QTreeWidgetItem *item = ui->treeWidget->topLevelItem(j);

   for (int i =0;i<item->childCount();i++)
        {
            QTreeWidgetItem *item_child = item->child(i);
            item_child->setHidden(false);
        }
    }

}

void ImageVideoManagerWidget::on_pushButton_clicked()
{
    bool flag=0,ok;
    settree_show();
    int time = ui->timelabel->text().toInt(&ok,10);
    for(int j=0;j<videocount+imagecount;j++)
    {
   QTreeWidgetItem *item = ui->treeWidget->topLevelItem(j);

   for (int i =0;i<item->childCount();i++)
        {

            QTreeWidgetItem *item_child = item->child(i);
            QString list ;
            bool check_time =check_item_time(item_child,time);
            if (check_time==true)
            {
                item->setHidden(false);
                flag = 1;
                item_child->setHidden(false);
                list = "in if";
                qDebug()<<list<<"\n";

            }
            else
            {
                item_child->setHidden(true);
                if (flag==0)
                    item->setHidden(true);
                list = "in else";
                qDebug()<<list<<"\n";
            }


          //  list = item_child->toolTip(1)+item_child->toolTip(0);
          //  qDebug()<<list<<"\n";


        }
   flag = 0;
    }

   /* while (item->child(i)) {
               //if ((*it)->toolTip(1).toInt(&ok,10) <= time)

                   ui->image_view->setText(ui->image_view->text()+","+(*it)->toolTip(1));
                   //ui->image_view->setText(ui->image_view->text()+","+(*it)->toolTip(0));
                 //(*it)->setHidden(false);
              // else
                   //(*it)->setHidden(true);
                 //  ROS_ERROR("more");
           it++;

        }

}*/





void ImageVideoManagerWidget::on_cameralist_currentIndexChanged(int index)
{
    int toplevelitemcount = ui->treeWidget->topLevelItemCount();
    QString camera;
    QTreeWidgetItem * item;
    for(int k=0;k<toplevelitemcount;k++)
    {
        ui->treeWidget->topLevelItem(k)->setHidden(false);
    }
    switch(index)
    {
    case 0: camera="/l_image_full";break;
    case 1: camera = "/r_image_full"; break;
    case 2: camera = "/lhl_image_full"; break;
    case 3: camera = "/lhr_image_full"; break;
    case 4: camera = "/rhr_image_full";break;
    case 5: camera = "/rhl_image_full";break;
    }
    for (int i =0;i<toplevelitemcount;i++)
    {
        if(index==6)
            for(int k=0;k<toplevelitemcount;k++)
            {
                ui->treeWidget->topLevelItem(k)->setHidden(false);
            }
        else{
        //ui->image_view->setText(ui->treeWidget->topLevelItem(i)->text(2));
        if(ui->treeWidget->topLevelItem(i)->text(2)!=camera)
        {
            item=ui->treeWidget->topLevelItem(i);
            item->setHidden(true);
           // ui->image_view->setText(ui->treeWidget->topLevelItem(i)->text(2));

        }
        }
    }
}