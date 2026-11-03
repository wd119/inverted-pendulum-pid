#include "stm32f10x.h"
#include "PID.h"
#include <math.h>
// Device header
void PID_update(PID_t* pid){
	pid->error1=pid->error0;
	pid->error0=pid->Target-pid->Actual;

	
	
//	if (pid->Ki != 0)					//如果Ki不为0
//	{
//		/* 积分分离：误差过大时暂停积分，避免大偏差下积分饱和 */
//		if (fabsf(pid->error0) < PID_INT_SEP)
//		{
//			/* 变速积分：误差越大，积分速度越慢 */
//			float C = 1 / (0.5f * fabsf(pid->error0) + 1);
//			pid->errorInt += C * pid->error0;
//			/* 积分限幅 */
//			if (pid->errorInt >  500) {pid->errorInt =  500;}
//			if (pid->errorInt < -500) {pid->errorInt = -500;}
//		}
//	}
//	else							//否则
//	{
//		pid->errorInt = 0;			//误差积分直接归0
//	}
//	if (p->Ki != 0)					//如果Ki不为0
	if (pid->Ki != 0)					//如果Ki不为0
	{
		pid->errorInt += pid->error0;	//进行误差积分
	}
	else							//否则
	{
		pid->errorInt = 0;			//误差积分直接归0
	}
	pid->Out=pid->Kp*pid->error0+pid->Ki*pid->errorInt+pid->Kd*(pid->error0-pid->error1);

	if(pid->Out>pid->MaxOut){	pid->Out=pid->MaxOut;}
	if(pid->Out<pid->MinOut){	pid->Out=pid->MinOut;}
}
