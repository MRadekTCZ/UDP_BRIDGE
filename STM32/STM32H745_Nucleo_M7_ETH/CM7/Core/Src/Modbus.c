#include "Modbus.h"
void updateCurrentTime(struct Time *currentTime, unsigned short int milliseconds)
{
    currentTime->miliSeconds += milliseconds;

    if (currentTime->miliSeconds >= 1000)
    {
        currentTime->miliSeconds -= 1000;
        currentTime->seconds++;

        if (currentTime->seconds >= 60)
        {
            currentTime->seconds = 0;
            currentTime->minutes++;

            if (currentTime->minutes >= 60)
            {
                currentTime->minutes = 0;
                currentTime->hours++;

                if (currentTime->hours >= 24)
                {
                    // Obliczamy, ile dni minęło od 1 sierpnia 2020 roku
                    unsigned int daysSinceStart = 0;
                    for (int year = 2020; year < 2023; year++)
                    {
                        daysSinceStart += (year % 4 == 0) ? 366 : 365;
                    }
                    // Dodajemy dni od początku roku do obecnego dnia
                    daysSinceStart += 213; // 1 sierpnia to dzień 213 w roku 2020

                    // Obliczamy ile dni minęło
                    unsigned int totalDays = daysSinceStart + (currentTime->hours / 24);
                    currentTime->hours %= 24;

                    // Obliczamy rok, miesiąc i dzień na podstawie liczby dni
                    int year = 2020;
                    while (1)
                    {
                        int daysInYear = (year % 4 == 0) ? 366 : 365;
                        if (totalDays < daysInYear)
                            break;
                        totalDays -= daysInYear;
                        year++;
                    }

                    // Obliczamy miesiąc i dzień na podstawie liczby dni
                    int month, day;
                    int daysInMonth;
                    for (month = 1; month <= 12; month++)
                    {
                        if (month == 2 && (year % 4 == 0))
                            daysInMonth = 29;
                        else
                        {
                            static const int daysInMonthArray[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
                            daysInMonth = daysInMonthArray[month - 1];
                        }
                        if (totalDays < daysInMonth)
                            break;
                        totalDays -= daysInMonth;
                    }
                    day = totalDays + 1; // +1, ponieważ dni zaczynają się od 1

                    // Aktualizujemy datę
                    //year - rok
                    //month - miesiąc (1 - 12)
                    //day - dzień (1 - 31)
                }
            }
        }
    }
};
