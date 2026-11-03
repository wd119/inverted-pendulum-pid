#include "stm32f10x.h"
#include "Delay.h"
#include "OLED.h"
#include "LED.h"
#include "Timer.h"
#include "Key.h"
#include "RP.h"
#include "Motor.h"
#include "Encoder.h"
#include "Serial.h"
#include "AD.h"
#include "PID.h"
#define CENTER_ANGLE	2083
#define	CENTER_RANGE	500
#define START_PWM	35
#define START_TIME	100
uint8_t keyNum;
uint8_t RunState;
uint16_t angle;
int16_t speed,location;
PID_t AnglePID={
	.Target=CENTER_ANGLE,
	
	.Kp=0.3,
	.Ki=0.02,
	.Kd=0.66,
	
	.MaxOut=100,
	.MinOut=-100,
	
};
PID_t locationPID={
	.Target=0,
	
	.Kp=0.4,
	.Ki=0,
	.Kd=4,
	
	.MaxOut=100,
	.MinOut=-100,
	
};
int main(void){

	OLED_Init();
	LED_Init();
	Key_Init();
	RP_Init();
	Motor_Init();
	Encoder_Init();
	Serial_Init();
	AD_Init();
	
	Timer_Init();
	while(1){
		keyNum=Key_GetNum();
		if(keyNum==1){
			if(RunState==0){
			RunState=21;
			}
			else{
				RunState=0;
			}
		}
		if(keyNum==2){
			locationPID.Target+=408;
			if(locationPID.Target>4080){
				locationPID.Target=4080;
			}
		}
		if(keyNum==3){
			locationPID.Target-=408;
			if(locationPID.Target<-4080){
				locationPID.Target=-4080;
			}
		}
		
		if(RunState){
			LED_ON();
		}else{
		LED_OFF();}
//		AnglePID.Kp=RP_GetValue(1)/4095.0*1;
//		AnglePID.Ki=RP_GetValue(2)/4095.0*0.2;
//		AnglePID.Kd=RP_GetValue(3)/4095.0*1;
//		
//		locationPID.Kp=RP_GetValue(1)/4095.0*1;
//		locationPID.Ki=RP_GetValue(2)/4095.0*1;
//		locationPID.Kd=RP_GetValue(3)/4095.0*9;
		
		OLED_Printf(42,0,OLED_6X8,"%02d",RunState);
		OLED_Printf(0,0,OLED_6X8,"Angle:");
		OLED_Printf(0,12,OLED_6X8,"Kp:%05.3f",AnglePID.Kp);
		OLED_Printf(0,20,OLED_6X8,"Ki:%05.3f",AnglePID.Ki);
		OLED_Printf(0,28,OLED_6X8,"Kd:%05.3f",AnglePID.Kd);
		OLED_Printf(0,40,OLED_6X8,"Tar:%04.0f",AnglePID.Target);
		OLED_Printf(0,48,OLED_6X8,"Act:%04d",angle);
		OLED_Printf(0,56,OLED_6X8,"Out:%+04.0f",AnglePID.Out);
		
		OLED_Printf(64,12,OLED_6X8,"Kp:%05.3f",locationPID.Kp);
		OLED_Printf(64,20,OLED_6X8,"Ki:%05.3f",locationPID.Ki);
		OLED_Printf(64,28,OLED_6X8,"Kd:%05.3f",locationPID.Kd);
		OLED_Printf(64,40,OLED_6X8,"Tar:%+05.0f",locationPID.Target);
		OLED_Printf(64,48,OLED_6X8,"Act:%+05d",location);
		OLED_Printf(64,56,OLED_6X8,"Out:%+04.0f",locationPID.Out);
		OLED_Update();
		Serial_Printf("%f,%d,%f,%f\r\n",AnglePID.Target,angle,AnglePID.Out,AnglePID.errorInt);
	}
}
void TIM1_UP_IRQHandler(void){
	static uint16_t Count0,Count1,Count2,CountTime;
	static uint16_t Angle0,Angle1,Angle2;
	static float loc_out_f;
    if(TIM_GetITStatus(TIM1,TIM_IT_Update)==SET){
        Key_Tick();

		angle=AD_GetValue();
		speed=Encoder_Get();
		location+=speed;
		
		if(RunState==0){
			Motor_SetPWM(0);
		}else if(RunState==1){
				//判断状态
			Count0++;
			if(Count0>=40){
				Count0=0;
				
				Angle2=Angle1;
				Angle1=Angle0;
				Angle0=angle;
				
				if(Angle0>CENTER_ANGLE+CENTER_RANGE&&Angle1>CENTER_ANGLE+CENTER_RANGE&&Angle2>CENTER_ANGLE+CENTER_RANGE&&
					Angle1<Angle0&&Angle1<Angle2){
					RunState=31;
				}
				if(Angle0<CENTER_ANGLE+CENTER_RANGE&&Angle1<CENTER_ANGLE+CENTER_RANGE&&Angle2<CENTER_ANGLE+CENTER_RANGE&&
					Angle1>Angle0&&Angle1>Angle2){
					RunState=21;
				}
					if(Angle0>CENTER_ANGLE-CENTER_RANGE&&Angle0<CENTER_ANGLE+CENTER_RANGE&&
						Angle1>CENTER_ANGLE-CENTER_RANGE&&Angle1<CENTER_ANGLE+CENTER_RANGE){
							location=0;
							AnglePID.errorInt=0;
							RunState=4;
					}
			}
		}else if(RunState==21){//2x向右施力
			Motor_SetPWM(START_PWM);
			CountTime=START_TIME;
			RunState=22;
		}else if(RunState==22){
			CountTime--;
			if(CountTime==0){
				RunState=23;
			}
		}else if(RunState==23){
			Motor_SetPWM(-START_PWM);
			CountTime=START_TIME;
			RunState=24;
		}else if(RunState==24){
			CountTime--;
			if(CountTime==0){
				Motor_SetPWM(0);
				RunState=1;
			}
		}else if(RunState==31){//3x向右施力
			Motor_SetPWM(-START_PWM);
			CountTime=START_TIME;
			RunState=32;
		}else if(RunState==32){
			CountTime--;
			if(CountTime==0){
				RunState=33;
			}
		}else if(RunState==33){
			Motor_SetPWM(START_PWM);
			CountTime=START_TIME;
			RunState=34;
		}else if(RunState==34){
			CountTime--;
			if(CountTime==0){
				Motor_SetPWM(0);
				RunState=1;
			}
		}else if(RunState==4){
			if(!(angle>CENTER_ANGLE-CENTER_RANGE&&angle<CENTER_ANGLE+CENTER_RANGE)){
			RunState=0;
		}
			Count1++;
			if(Count1>=5){
				Count1=0;
				AnglePID.Actual=angle;
				PID_update(&AnglePID);
				Motor_SetPWM(-AnglePID.Out);
			}
			Count2++;
			if(Count2>=50){
				Count2=0;
				locationPID.Actual=location;
				PID_update(&locationPID);
				/* 外环输出一阶低通，抑制 Target 台阶跳变引起的内环抖动 */
				loc_out_f = 0.3f * locationPID.Out + 0.7f * loc_out_f;
				AnglePID.Target=CENTER_ANGLE-loc_out_f;
			}
		}
		
		
        TIM_ClearITPendingBit(TIM1,TIM_IT_Update);
    }
}
