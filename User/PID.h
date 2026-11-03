#ifndef __PID_H
#define __PID_H

/* 积分分离阈值：误差绝对值大于该值时暂停积分 */
#define PID_INT_SEP	150.0f

typedef struct{
	float Target;
	float Actual;
	float Out;
	float Kp;
	float Ki;
	float Kd;
	float error0;
	float error1;
	float errorInt;

	float MaxOut;
	float MinOut;
}PID_t;

void PID_update(PID_t* pid);


#endif
