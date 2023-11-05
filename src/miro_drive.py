#!/usr/bin/env python3
#-*- coding: utf-8 -*-
import rospy, time
from sensor_msgs.msg import LaserScan
from xycar_msgs.msg import xycar_motor

motor_msg = xycar_motor()
distance = []
pub = None

def callback(data):
    global distance, motor_msg
    distance = data.ranges

#motor_msg에 speed, angle 담아서 전송    
def drive_go(Speed, Angle):
    global motor_msg, pub
    motor_msg.speed = Speed
    motor_msg.angle = Angle
    pub.publish(motor_msg)
    

def main():
    global pub
    rospy.init_node('miro_drive')
    rospy.Subscriber('/scan', LaserScan, callback, queue_size = 1)
    pub = rospy.Publisher('xycar_motor', xycar_motor, queue_size = 1)
    time.sleep(3) #ready to connect lidar
    while not rospy.is_shutdown():
        
        front = 0
        front_left = 0
        front_right = 0
        left = 0
        right = 0
        fflag = False
        frflag = False
        flflag = False

        #각 5구간을 나눠서 진행(front, left. right)
        # front
        for degree in range(0,30):
            if distance[degree] <= 0.3 and distance[degree] != 0: #30cm 이내 진입시, 방해물이 64m 넘을 때 0값으로 들어와서 distance[degree] != 0로 오류 방지
                front += 1
            if front > 3: #해당 값 이내에 3번 진입시 front flag on
                fflag = True
                break
        # front_left
        for degree in range(-60,0):
            if distance[degree] <= 0.2 and distance[degree] != 0:
                front_left += 1
            if front_left > 3:
                flflag = True
                break
        #front_right
        for degree in range(30,90):
            if distance[degree] <= 0.2 and distance[degree] != 0:
                front_right += 1
            if front_right > 3:
                frflag = True
                break
        #right    
        for degree in range(90,120):
            if distance[degree] <= 0.3 and distance[degree] != 0: #30cm 이내 진입시, 방해물이 64m 넘을 때 0값으로 들어와서 distance[degree] != 0로 오류 방지
                right += 1
            if right > 3: #해당 값 이내에 3번 진입시 front flag on
                rflag = True
                break
        #left
        for degree in range(-90,-60):
            if distance[degree] <= 0.3 and distance[degree] != 0: #30cm 이내 진입시, 방해물이 64m 넘을 때 0값으로 들어와서 distance[degree] != 0로 오류 방지
                left += 1
            if left > 3: #해당 값 이내에 3번 진입시 front flag on
                lflag = True
                break

        #front_flag, left_flag, right_flag의 조합으로 8가지 경우로 나눔
        #case1 방해물 없는 경우 :직진
        if fflag == False and frflag == False and flflag == False:
            drive_go(10, 0)
        #case2 전면만 막힌경우: 우회전으로 후진
        elif fflag == True and frflag == False and flflag == False:
            drive_go(-10, 20)
        #오른쪽 방해물 있는 경우: 좌회전으로 전진    
        elif fflag == False and frflag == True and flflag == False:
            drive_go(10, -30)
        #왼쪽 방해물 있는 경우: 우회전으로 전진
        elif fflag == False and frflag == False and flflag == True:
            drive_go(10, 30)
        #전면과 오른쪽에 방해물 있는 경우: 좌회전으로 후진    
        elif fflag == True and frflag == True and flflag == False:
            drive_go(-10, -20)
        #전면과 왼쪽에 방해물 있는 경우: 우회전으로 후진
        elif fflag == True and frflag == False and flflag == True:
           drive_go(-10, 20)
        #전면, 왼쪽, 좌측, 방해물이 있는 경우: 직선으로 후진
        elif fflag == True and frflag == True and flflag == True:
            if rflag == False and lflag == True:
                drive_go(10, 50)
            elif rflag == True and lflag == False:
                drive_go(10,-50)
            else:
                drive_go(-10, 0)



if __name__ == '__main__':
    main()