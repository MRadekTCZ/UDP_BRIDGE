struct Time
{
	unsigned short int hours;
	unsigned short int minutes;
	unsigned short int seconds;
	unsigned short int miliSeconds;
};


struct Modbus_reg
{
	unsigned short int setvoltage;
	unsigned short int sin_current;
	unsigned short int counter_bt1;
	struct Time currentTime_modbus;
};
void updateCurrentTime(struct Time *currentTime, unsigned short int milliseconds);
