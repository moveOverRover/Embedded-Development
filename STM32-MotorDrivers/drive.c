// UGRT 2021 Motor Drivers For STM32 F4 arm boards
// Version: 0
// Author: Alex Semenov

#include "drive.h"

int pwmTest(Driver *driver){ // writes an increasing duty cycle to all channels for testing purposes
	if (driver == NULL){ return 1; }
	for(int i=0; i<100; i++){
		for(int j=0; j<4; j++){
			if (driver->channelManager[j] != 'z'){
				writeDuty(driver, j+1, (float)i);
			}
		}
	  	HAL_Delay(50);
	}
	return 0;
}

Driver *createDriver(TIM_HandleTypeDef *htim, int timer){
	Driver *driver = malloc(sizeof(Driver));	// creates driver object
	if (driver == NULL){ return NULL; }
	driver->channelManager = malloc(4 * sizeof(char));	// creates array of possible channels
	if (driver->channelManager == NULL){ return NULL; }

	driver->htim = htim;	// saves the timer object
	for (int i=0; i<4; i++){	// sets all channels to closed
		driver->channelManager[i] = 'z';
	}
	driver->timer = timer;	// saves the timer number
	if (assignTIM(driver) == 1){ return NULL; }	// saves the TIM register memory location
	return driver;
}

void destroyDriver(Driver *driver){ // frees the driver object
	if (driver == NULL){
		if (driver->channelManager != NULL){
			free(driver->channelManager);
		}
		free(driver->channelManager);
	}
	free(driver);
}

int attachMotor(Driver *driver, char identity, int channel){
	if (driver == NULL){ return 1; }
	driver->channelManager[channel-1] = identity;
	return 0;
}

int startAll(Driver *driver){ // starts PWM on all open channels
	if (driver == NULL){ return 1; }
	for(int i=0; i<4; i++){
		if (driver->channelManager[i] != 'z'){
			if (startPWM(driver, i+1) == 1){ return 1; }
		}
	}
	return 0;
}

int startPWM(Driver *driver, int channel){ // starts PWM on a given channel
	if (driver == NULL){ return 1; }
	switch (channel) {
		case 1: HAL_TIM_PWM_Start(driver->htim, TIM_CHANNEL_1); break;
		case 2: HAL_TIM_PWM_Start(driver->htim, TIM_CHANNEL_2); break;
		case 3: HAL_TIM_PWM_Start(driver->htim, TIM_CHANNEL_3); break;
		case 4: HAL_TIM_PWM_Start(driver->htim, TIM_CHANNEL_4); break;
		default: return 1;
	}
	return 0;
}

int stopAll(Driver *driver){ // stops PWM on all open channels
	if (driver == NULL){ return 1; }
	for(int i=0; i<4; i++){
		if (driver->channelManager[i] != 'z'){
			if (stopPWM(driver, i+1) == 1){ return 1; }
		}
	}
	return 0;
}

int stopPWM(Driver *driver, int channel){ // stops PWM on a given channel
	if (driver == NULL){ return 1; }
	switch (channel) {
		case 1: HAL_TIM_PWM_Stop(driver->htim, TIM_CHANNEL_1); break;
		case 2: HAL_TIM_PWM_Stop(driver->htim, TIM_CHANNEL_2); break;
		case 3: HAL_TIM_PWM_Stop(driver->htim, TIM_CHANNEL_3); break;
		case 4: HAL_TIM_PWM_Stop(driver->htim, TIM_CHANNEL_4); break;
		default: return 1;
	}
	return 0;
}


int writeDuty(Driver *driver, int channel, float duty){ // changes the duty cycle of the PWM
	// servo frequency is usually 50Hz; 1/50 =.02 = 20ms; servos duty cycle is usually 1-2ms so 5-10 %duty to control a servo
	if (driver == NULL){ return 1; }
	switch (channel) {
		case 1: driver->TIM->CCR1 = duty; break;
		case 2: driver->TIM->CCR2 = duty; break;
		case 3: driver->TIM->CCR3 = duty; break;
		case 4: driver->TIM->CCR4 = duty; break;
		default: return 1;
	}
	return 0;
}

int servoWrite(Driver *driver, char identity, int deg){ // moves the servos from 0-180 deg
	if (driver == NULL){ return 1; }
	if (deg < 0 || deg > 180){ return 1; }
	float duty = mapfloat((float)deg, 0.0, 180.0, 5.0, 10.0);
	int channel = getChannel(driver, identity);
	if (channel == 0){ return 1; }
	if (writeDuty(driver, channel, duty) == 1){ return 1; }
	return 0;
}

int motorWrite(Driver *driver, char identity, int powr){ // writes to motor PWM to a channel
	if (powr < 0 || powr > 100){ return 1; }
	int duty = mapInt(powr, 0, 100, 0, 101);
	int channel = getChannel(driver, identity);
	if (channel == 0){ return 1; }
	if (writeDuty(driver, channel , (float)duty) == 1){ return 1; }
	return 0;
}

int assignTIM(Driver *driver){	// TIM1-14 are macros for memory locations of registers or something, this helper functions saves them
	if (driver == NULL){ return 1; }
	switch (driver->timer) {
		case 1: driver->TIM = TIM1; break;
		case 2: driver->TIM = TIM2; break;
		case 3: driver->TIM = TIM3; break;
		case 4: driver->TIM = TIM4; break;
		case 5: driver->TIM = TIM5; break;
		case 6: driver->TIM = TIM6; break;
		case 7: driver->TIM = TIM7; break;
		case 8: driver->TIM = TIM8; break;
		case 9: driver->TIM = TIM9; break;
		case 10: driver->TIM = TIM10; break;
		case 11: driver->TIM = TIM11; break;
		case 12: driver->TIM = TIM12; break;
		case 13: driver->TIM = TIM13; break;
		case 14: driver->TIM = TIM14; break;
		default : return 1;
	}
	return 0;
}

int getChannel(Driver *driver, char identity){ // gets the channel from the identity
	for(int i=0; i<4; i++){
		if (driver->channelManager[i] == identity){
			return i+1;
		}
	}
	return 0;
}

float mapfloat(float x, float in_min, float in_max, float out_min, float out_max){
	return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

int mapInt(int x, int in_min, int in_max, int out_min, int out_max){
	return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}


