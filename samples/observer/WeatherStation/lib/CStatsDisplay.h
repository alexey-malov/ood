#ifndef CSTATSDISPLAY_H
#define CSTATSDISPLAY_H

#include "IObserver.h"
#include "SWeatherInfo.h"
#include <iostream>

class CStatsDisplay : public IObserver<SWeatherInfo>
{
private:
	/* Метод Update сделан приватным, чтобы ограничить возможность его вызова напрямую
	Классу CObservable он будет доступен все равно, т.к. в интерфейсе IObserver он
	остается публичным
	*/
	void Update(SWeatherInfo const& data) override
	{
		if (m_minTemperature > data.temperature)
		{
			m_minTemperature = data.temperature;
		}
		if (m_maxTemperature < data.temperature)
		{
			m_maxTemperature = data.temperature;
		}
		m_accTemperature += data.temperature;
		++m_countAcc;

		std::cout << "Max Temp " << m_maxTemperature << std::endl;
		std::cout << "Min Temp " << m_minTemperature << std::endl;
		std::cout << "Average Temp " << (m_accTemperature / m_countAcc) << std::endl;
		std::cout << "----------------" << std::endl;
	}

	double m_minTemperature = std::numeric_limits<double>::infinity();
	double m_maxTemperature = -std::numeric_limits<double>::infinity();
	double m_accTemperature = 0;
	unsigned m_countAcc = 0;
};

#endif // CSTATSDISPLAY_H
