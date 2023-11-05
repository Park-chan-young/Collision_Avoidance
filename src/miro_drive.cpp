#include <ros/ros.h>
#include <sensor_msgs/LaserScan.h>
#include <xycar_msgs/xycar_motor.h>
#include <time.h>
#include <vector>
#include <random>
#include <cmath>
#include <iostream>
#include <numeric>

using namespace std;

vector<float> Dist;
int avg_val[5]; //직선 ,fl, fr, l, r
int dic_ang[5] = { 0, -50, 50, -50, 50};
ros::Publisher pub;

//평균
void cal_avg(int (&ref)[5]){
    //front
    int fnum = 42;
    int fsum = 0;
    for(int i = -21; i < 21; i++){
        if(Dist[i] == 0){
            fnum--;
        }
        else{
            fsum += Dist[i];
        }
    }
    ref[0] = fsum/fnum;
    //front_left
    int flnum = 84;
    int flsum = 0;
    for(int i = -105; i < -21; i++){
        if(Dist[i] == 0){
            flnum--;
        }
        else{
            flsum += Dist[i];
        }
    }
    ref[1] = flsum/flnum;
    //front_right
    int frnum = 84;
    int frsum = 0;
    for(int i = 21; i < 105; i++){
        if(Dist[i] == 0){
            frnum--;
        }
        else{
            frsum += Dist[i];
        }
    }
    ref[2] = frsum/frnum;
    //left
    int lnum = 42;
    int lsum = 0;
    for(int i = -147; i < -105; i++){
        if(Dist[i] == 0){
            lnum--;
        }
        else{
            lsum += Dist[i];
        }
    }
    ref[3] = lsum/lnum;
    //right
    int rnum = 42;
    int rsum = 0;
    for(int i = 105; i < 147; i++){
        if(Dist[i] == 0){
            rnum--;
        }
        else{
            rsum += Dist[i];
        }
    }
    ref[4] = rsum/rnum;
}



// 1.8 이내 1.8로 반환
/*
void change_val()
{
    for (int i = 0; i < Dist.size(); i++)
    {
        if (Dist[i] <= 1.8)
        {
            Dist[i] = 1.8;
        }
    }
    return;
}
*/

void callback(const sensor_msgs::LaserScan::ConstPtr& data)
{
    Dist = data->ranges; //.begin(), data->ranges.end());
    //std::cout<< Dist.size() <<"\n"
}

// motor_msg에 speed, angle 담아서 전송
void drive_go (int Speed, int Angle)
{
    xycar_msgs::xycar_motor msg;

    msg.speed = Speed;
    msg.angle = Angle;
    pub.publish(msg);
}

int main(int argc, char ** argv)
{
    //change_val(); // 1.8 이하의 값들을 1.8로!! (64m 넘어갈 때 0으로 반환하는 것과 차이를 두기 위해)
    ros::init(argc, argv, "miro_drive");
    ros::NodeHandle nh;
    ros::Subscriber sub = nh.subscribe("/scan", 1, callback);
    pub = nh.advertise<xycar_msgs::xycar_motor>("xycar_motor", 1);
    ros::Duration(3).sleep(); // ready to connect lidar
    
    while(ros::ok())
    {
        ros::spinOnce();
        
        cal_avg(avg_val);
        
        int front = 0;
        int front_left = 0;
        int front_right = 0;
        int left = 0;
        int right = 0;
        bool fflag = false;
        bool frflag = false;
        bool flflag = false;
        bool lflag = false;
        bool rflag = false;

        // 각 5구간을 나눠서 진행 (front, left, right)
        // front
        for(int i =361; i < 400; i++){
            cout << Dist[i] << " ";
        }
        cout <<"\n" << "---------------------------------------------------------------------------------------------" <<"\n";
        for (int degree = 484; degree <505; degree++)
        {
            if (Dist[degree] <= 0.3 && Dist[degree] != 0)
            {
                front += 1;
            }
            if (front > 3)
            {
                fflag = true;
                break;
            }
        }
        for (int degree = 0; degree < 21; degree++)
        {   
            
            if (Dist[degree] <= 0.45 && Dist[degree] != 0)
            {
                front += 1;
            }
            if (front > 3)
            {
                fflag = true;
                break;
            }
        }
        // front_left
        for (int degree = 400; degree < 484; degree++)
        {   
            if (Dist[degree] <= 0.45 && Dist[degree] != 0)
            {
                front_left += 1;
            }
            if (front_left > 3)
            {
                flflag = true;
                break;
            }
        }
        // front_right
        for (int degree = 21; degree < 105; degree++)
        {
            if (Dist[degree] <= 0.4 && Dist[degree] != 0)
            {
                front_right += 1;
            }
            if (front_right > 3)
            {
                frflag = true;
                break;
            }
        }
        // right
        for (int degree = 105; degree < 147; degree++)
        {
            if (Dist[degree] <= 0.4 && Dist[degree] != 0)
            {
                right += 1;
            }
            if (right > 3)
            {
                rflag = true;
                break;
            }
        }
        // left
        for (int degree = 358; degree < 400; degree++)
        {
            if (Dist[degree] <= 0.4 && Dist[degree] != 0)
            {
                left += 1;
            }
            if (left > 3)
            {
                lflag = true;
                break;
            }
        }
        if(fflag == false && frflag == false && flflag == false){
            drive_go(10, 0); 
            ROS_INFO("case: 1");
        }
        else if(fflag== true && frflag== false &&flflag ==false){
            int ret = -1;
            int d= 0;
            for(int i =0; i < 5; i++){
                if(avg_val[i] == 0){
                    d = i;
                    break;
                }
                if(ret <= avg_val[i]){
                    d = i;
                    ret = avg_val[i];
                }
                else{
                    ;
                }
            }
            drive_go(-10,-dic_ang[d]);
            ROS_INFO("case: 2");
            ros::Rate(1).sleep();
        }

        else if(fflag == false && frflag == true && flflag == false){
            drive_go(12, 50);
            ROS_INFO("case: 3");
        }
        else if(fflag == false && frflag == false && flflag == true){
            drive_go(12, -50);
            ROS_INFO("case: 4");
        }
        else if(fflag == true && frflag == true && flflag == false){
            drive_go(-10, -50);
            ros::Rate(1).sleep();
            ROS_INFO("case: 5");
        }
        
        else if(fflag == true && frflag == false && flflag == true){
            
            drive_go(-10, 50);
            ros::Rate(1).sleep();
            ROS_INFO("case: 6");
        }
        else if(fflag == true && frflag == true && flflag == true){
            
            if(rflag == false && lflag == true){
                drive_go(15, -50);
                ROS_INFO("case: 7");
            }
            else if(rflag == true && lflag == false){
                drive_go(15,50);
                ROS_INFO("case: 8");
            }
            else{
                ROS_INFO("case: nothing");
                int rret = -1;
                int dd= 0;
                for(int i =0; i < 5; i++){
                    if(avg_val[i] == 0){
                        dd = i;
                        break;
                    }
                    if(rret <= avg_val[i]){
                        dd = i;
                        rret = avg_val[i];
                    }
                    else{
                        ;
                    }
                }
                drive_go(-10,-dic_ang[dd]);
                ROS_INFO("case: 2");
                ros::Rate(1).sleep();
            }
            
        }
    }
    return 0;
}
