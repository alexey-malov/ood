#include <boost/noncopyable.hpp>
#include <boost/signals2.hpp>
#include <functional>
#include <iostream>
#include <map>

using namespace std;
namespace ph = std::placeholders;

namespace signals = boost::signals2;
using signals::scoped_connection;
using signals::signal;

class ICurrencyRate
{
public:
	using RateSignal = signal<void(double rate)>;

	[[nodiscard]] virtual scoped_connection DoOnRateChange(const RateSignal::slot_type& slot) = 0;
	[[nodiscard]] virtual double GetRate() const = 0;

	virtual ~ICurrencyRate() = default;
};

class Stock : public ICurrencyRate
{
public:
	void SetRate(double rate)
	{
		if (m_rubToUSD != rate)
		{
			m_rubToUSD = rate;
			m_rateChangeSignal(rate);
		}
	}

	[[nodiscard]] double GetRate() const override
	{
		return m_rubToUSD;
	}

	[[nodiscard]] scoped_connection DoOnRateChange(const RateSignal::slot_type& slot) override
	{
		return m_rateChangeSignal.connect(slot);
	}

private:
	double m_rubToUSD = 59.0;
	RateSignal m_rateChangeSignal;
};

class AverageCurrencyRateMonitor
{
public:
	AverageCurrencyRateMonitor(ICurrencyRate& cr)
	{
		m_rateChangeConnection = cr.DoOnRateChange(bind_front(&AverageCurrencyRateMonitor::OnCurrencyRateChange, this));
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
	scoped_connection m_rateChangeConnection;
};

int main()
{
	Stock s;

	auto conn = s.DoOnRateChange([](double rate) {
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
	s.SetRate(90);
	s.SetRate(50);
}