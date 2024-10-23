#include <functional>
#include <iostream>
#include <map>

using namespace std;
namespace ph = std::placeholders;

class ICurrencyRate
{
public:
	// Наблюдатель - функция, получающая сигнал об изменении курса валюты
	using RateObserver = function<void(double rate)>;

	using Token = uint64_t;

	[[nodiscard]] virtual Token DoOnRateChange(const RateObserver& observer) = 0;
	virtual void RemoveRateChangeObserver(Token token) = 0;
	[[nodiscard]] virtual double GetRate() const = 0;

protected:
	~ICurrencyRate() = default;
};

class Stock : public ICurrencyRate
{
public:
	Stock() = default;

	// Запрещаем копирование и перемещение объектов Stock
	Stock(const Stock&) = delete;
	Stock& operator=(const Stock&) = delete;

	void SetRate(double rate)
	{
		if (m_rubToUSD != rate)
		{
			m_rubToUSD = rate;
			for (auto& observer : m_observers)
			{
				observer.second(m_rubToUSD);
			}
		}
	}

	[[nodiscard]] Token DoOnRateChange(const RateObserver& observer) override
	{
		while (!m_observers.try_emplace(++m_nextToken, observer).second)
			;

		return m_nextToken;
	}

	[[nodiscard]] double GetRate() const override
	{
		return m_rubToUSD;
	}

	void RemoveRateChangeObserver(Token token) override
	{
		m_observers.erase(token);
	}

private:
	double m_rubToUSD = 59.0;
	Token m_nextToken = 0;
	map<Token, RateObserver> m_observers;
};

class AverageCurrencyRateMonitor
{
public:
	AverageCurrencyRateMonitor(ICurrencyRate& cr)
		: m_currencyRate(cr)
	{
		m_rateChangeToken = cr.DoOnRateChange([this](double rate) {
			OnCurrencyRateChange(rate);
		});
		//m_rateChangeToken = cr.DoOnRateChange(bind_front(&AverageCurrencyRateMonitor::OnCurrencyRateChange, this));
	}

	AverageCurrencyRateMonitor(const AverageCurrencyRateMonitor&) = delete;
	AverageCurrencyRateMonitor& operator=(const AverageCurrencyRateMonitor&) = delete;

	~AverageCurrencyRateMonitor()
	{
		m_currencyRate.RemoveRateChangeObserver(m_rateChangeToken);
	}

	[[nodiscard]] double GetAverageRate() const
	{
		return (m_count != 0) ? m_accRate / m_count : 0;
	}

private:
	void OnCurrencyRateChange(double newRate)
	{
		m_accRate += newRate;
		++m_count;
		cout << "Average rate is: " << GetAverageRate() << endl;
	}

	double m_accRate = 0.0;
	int m_count = 0;
	ICurrencyRate& m_currencyRate;
	ICurrencyRate::Token m_rateChangeToken;
};

int main()
{
	Stock s;

	auto token = s.DoOnRateChange([](double rate) {
		cout << "Rate is: " << rate << " RUR/USD" << endl;
	});

	s.SetRate(70);
	s.SetRate(59);
	s.SetRate(80);

	{
		AverageCurrencyRateMonitor avgRateMonitor(s);
		s.SetRate(60);
		s.SetRate(70);
		cout << "------------" << endl;
	}
	s.SetRate(80);
	// Отписались от получения уведомлений
	s.RemoveRateChangeObserver(token);
	// При следующих изменениях курса уведомления приходить не будут
	s.SetRate(90);
	s.SetRate(50);
}