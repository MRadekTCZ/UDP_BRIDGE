struct Time
{
	unsigned int hours;
	unsigned int minutes;
	unsigned int seconds;
	unsigned int miliSeconds;
};


struct Modbus_reg
{
	unsigned int setvoltage;
	unsigned int sin_current;
	unsigned int counter_bt1;
	struct Time currentTime_modbus;
};
void updateCurrentTime(struct Time *currentTime, unsigned int milliseconds);
