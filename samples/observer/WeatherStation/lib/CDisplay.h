#ifndef CDISPLAY_H
#define CDISPLAY_H

#include "IObserver.h"
#include "SWeatherInfo.h"
#include <iostream>

class CDisplay : public IObserver<SWeatherInfo>
{
private:
	/*
	 * Метод Update сделан приватным, чтобы ограничить возможность его вызова напрямую
	 * Классу CObservable он будет доступен все равно, т.к. в интерфейсе IObserver он
	 * остается публичным
	 */
	void Update(SWeatherInfo const& data) override
	{
		std::cout << "Current Temp " << data.temperature << std::endl;
		std::cout << "Current Hum " << data.humidity << std::endl;
		std::cout << "Current Pressure " << data.pressure << std::endl;
		std::cout << "----------------" << std::endl;
	}
};

#endif // CDISPLAY_H
