
/*Slk_PEC
Programmer : Santosh L kakad  (SVNIT surat M-TECH power electronics P15EL001)
Email ID - > kakadsantosh24@gmail.com	 
PROJECT : A variable duty ratio fixed frequnency signal for dc converters 
output pin is port E pin 2
*/

#include "stm32f4xx.h"
#include "arm_math.h" 
#define PWM_F 30000
GPIO_InitTypeDef  GPIO_InitStructure;
TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
TIM_OCInitTypeDef  TIM_OCInitStructure;
NVIC_InitTypeDef NVIC_InitStructure;
unsigned char key_flg=0;
uint16_t TimerPeriod = 0;
uint16_t Com_Value = 0;
void TIM1_Config(void);
void TIM3_Config(void);
void key_scan(void);
void GPIO_Config(void);
uint16_t PrescalerValue = 0;
uint8_t new_key,old_key,tran_cnt;
uint8_t action_flg=0;
uint8_t duty_cycle=10;  //<-- iintial duty ratio

int main(void)
{
	new_key=0x04;  //<- if key is connected in active low fashion
	old_key=0x04;
	tran_cnt=0;
	GPIO_Config();  
	TIM1_Config();   // TIM1-> CNT is used a sawtooth wave 
	TIM3_Config();  // Base timer for key scanning
	while (1)
		{
		if(key_flg!=0x00)   // at every 
		{
			key_flg=0;
			key_scan();
		}	
		if(action_flg ==1)
			{
			action_flg=0;
			duty_cycle+=5;
			if(duty_cycle>=90)
			duty_cycle=10;
			Com_Value = (uint16_t) (((uint32_t) duty_cycle * (TimerPeriod - 1)) / 100);
			TIM1->CCR1 = Com_Value;
			} 	
		}
}

void	GPIO_Config(void)
{
	//KEY is connected to Port E Pin 2 
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOE, &GPIO_InitStructure);  
}	

void TIM1_Config(void)
{

  	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE , ENABLE);
  	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1 , ENABLE);
  	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
  	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
  	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
 	  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP ;
 	  GPIO_Init(GPIOE, &GPIO_InitStructure);
   
  	GPIO_PinAFConfig(GPIOE, GPIO_PinSource9, GPIO_AF_TIM1);
	
	TimerPeriod = (SystemCoreClock / PWM_F ) - 1; // should be less than  65535
	Com_Value = (uint16_t) (((uint32_t) duty_cycle * (TimerPeriod - 1)) / 100);
	
	/* Time Base configuration */
	TIM_TimeBaseStructure.TIM_Prescaler = 0;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseStructure.TIM_Period = TimerPeriod;
	TIM_TimeBaseStructure.TIM_ClockDivision = 0;
	TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;
	TIM_TimeBaseInit(TIM1, &TIM_TimeBaseStructure);

	/* Channel 1 Configuration in PWM mode */
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM2;
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
	TIM_OCInitStructure.TIM_Pulse = Com_Value;
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_Low; //<- change it to  HIGH for complemetry 
	TIM_OC1Init(TIM1, &TIM_OCInitStructure);
	TIM_CtrlPWMOutputs(TIM1, ENABLE);

}

void TIM3_Config(void)
{

  	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
	TIM_TimeBaseStructure.TIM_Period = 41999;
	TIM_TimeBaseStructure.TIM_Prescaler = 1;
	TIM_TimeBaseStructure.TIM_ClockDivision = 0;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);
  	NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;
  	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  	NVIC_Init(&NVIC_InitStructure);
	
	TIM_Cmd(TIM3, ENABLE);

	TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);
}

void key_scan(void)
{
	new_key=(uint8_t)(GPIOE->IDR&GPIO_Pin_2);
	if(new_key!=old_key)
	{
		tran_cnt++;
		old_key=new_key;
		if(tran_cnt>=2)
		{
			tran_cnt=0;
			action_flg=1;
		}
	}
	
}

void TIM3_IRQHandler(void)
{
	TIM_ClearITPendingBit(TIM3,TIM_IT_Update);
	key_flg=0x01;
}


