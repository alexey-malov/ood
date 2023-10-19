#include <iostream>
#include <string>
#include <functional>
#include <tuple>
#include <unordered_map>
#include <stdexcept>
#include <sstream>
#include <format>

using namespace std;

enum class Direction
{
	North, West, South, East
};
struct IDuck
{
	virtual void Fly(Direction direction, int distance) = 0;
	virtual void Quack(const string & name) = 0;
	virtual int GetTotalFlyDistance() const = 0;
	virtual ~IDuck() = default;
};

string DirectionToString(Direction direction)
{
	return
		(direction == Direction::East) ? "east" :
		(direction == Direction::North) ? "north" :
		(direction == Direction::South) ? "south" : "west";
}

class CDuck : public IDuck
{
public:
	void Quack(const string & name) override
	{
		cout << "Quack Quack " << name << endl;
	}

	void Fly(Direction direction, int distance) override
	{
		cout << std::format("I am flying {} {} km.", DirectionToString(direction), distance) << endl;
		m_totalDistance += distance;
	}
	int GetTotalFlyDistance() const override
	{
		return m_totalDistance;
	}
private:
	int m_totalDistance = 0;
};

struct IInputPipe
{
	typedef function<string(const string& data)> MessageHandler;

	virtual void RegisterMessageReceiver(const string& url, const MessageHandler & handler) = 0;
	virtual void UnregisterMessageReceiver(const string& url) = 0;
	virtual ~IInputPipe() = default;
};

struct IOutputPipe
{
	virtual string SendMessage(const string& url, const string& data) = 0;
	virtual ~IOutputPipe() = default;
};

class CDuckStub
{
public:
	static const string QUACK_METHOD_ID;
	static const string FLY_METHOD_ID;
	static const string GET_TOTAL_FLY_DISTANCE_METHOD_ID;

	CDuckStub(IInputPipe & pipe, const string& id)
		: m_pipe(pipe)
	{
		using namespace std::placeholders;
		m_pipe.RegisterMessageReceiver(GetURL(id, QUACK_METHOD_ID), 
			bind(&CDuckStub::HandleQuack, this, _1));
		m_pipe.RegisterMessageReceiver(GetURL(id, FLY_METHOD_ID),
			bind(&CDuckStub::HandleFly, this, _1));
		m_pipe.RegisterMessageReceiver(GetURL(id, GET_TOTAL_FLY_DISTANCE_METHOD_ID), 
			bind(&CDuckStub::HandleGetTotalFlyDistance, this, _1));
	}
	~CDuckStub()
	{
		m_pipe.UnregisterMessageReceiver(GetURL(m_id, QUACK_METHOD_ID));
		m_pipe.UnregisterMessageReceiver(GetURL(m_id, FLY_METHOD_ID));
		m_pipe.UnregisterMessageReceiver(GetURL(m_id, GET_TOTAL_FLY_DISTANCE_METHOD_ID));
	}
	static string GetURL(const string& objectId, const string& methodName)
	{
		return "msg:CDuckStub/" + objectId + "/" + methodName;
	}
private:
	string HandleQuack(const string& data)
	{
		m_duck.Quack(data);
		return "";
	}
	string HandleFly(const string & data)
	{
		istringstream strm(data);
		string direction;
		int distance;
		if (strm >> direction >> distance)
		{
			m_duck.Fly(StringToDirection(direction), distance);
		}
		else
		{
			throw invalid_argument("Invalid data");
		}
		return "";
	}
	string HandleGetTotalFlyDistance(const string& data)
	{
		if (!data.empty())
		{
			throw invalid_argument("Not data is expected");
		}
		return to_string(m_duck.GetTotalFlyDistance());
	}
	static Direction StringToDirection(const string& direction)
	{
		return
			(direction == "east") ? Direction::East :
			(direction == "west") ? Direction::West :
			(direction == "south") ? Direction::South :
			(direction == "north") ? Direction::North :
			throw invalid_argument("unknown direction");
	}

	CDuck m_duck;
	string m_id;
	IInputPipe & m_pipe;
};
const string CDuckStub::QUACK_METHOD_ID = "Quack";
const string CDuckStub::FLY_METHOD_ID = "Fly";
const string CDuckStub::GET_TOTAL_FLY_DISTANCE_METHOD_ID = "GetTotalFlyDistance";

class CDuckProxy : public IDuck
{
public:
	CDuckProxy(IOutputPipe & pipe, const string& id)
		: m_pipe(pipe)
		, m_id(id)
	{
	}
	void Quack(const string & name) override
	{
		m_pipe.SendMessage(GetMethodURL(CDuckStub::QUACK_METHOD_ID), name);
	}
	void Fly(Direction direction, int distance) override
	{
		m_pipe.SendMessage(GetMethodURL(CDuckStub::FLY_METHOD_ID),
			DirectionToString(direction) + " " + to_string(distance));
	}
	int GetTotalFlyDistance() const override
	{
		return stoi(m_pipe.SendMessage(
			GetMethodURL(CDuckStub::GET_TOTAL_FLY_DISTANCE_METHOD_ID),
			""));
	}
private:
	string GetMethodURL(const string& methodName)const
	{
		return CDuckStub::GetURL(m_id, methodName);
	}
	IOutputPipe & m_pipe;
	string m_id;
};

void PlayWithDuck(IDuck & duck)
{
	duck.Quack("John");
	duck.Fly(Direction::North, 10);
	duck.Fly(Direction::East, 5);
	cout << "Total fly distance " << duck.GetTotalFlyDistance() << "km\n";
}


class CMockPipe : public IInputPipe, public IOutputPipe
{
public:
	void RegisterMessageReceiver(const string& url, const MessageHandler & handler) override
	{
		auto result = m_receivers.emplace(url, handler);
		if (!result.second)
		{
			throw invalid_argument("Message receiver has already been registered");
		}
	}
	void UnregisterMessageReceiver(const string& url) override
	{
		m_receivers.erase(url);
	}
	string SendMessage(const string& url, const string& data) override
	{
		return m_receivers.at(url)(data);
	}
private:
	unordered_map<string, MessageHandler> m_receivers;
};

int main()
{
	CMockPipe pipe;
	CDuckStub stub(pipe, "duck1");
	CDuckProxy proxy(pipe, "duck1");
	PlayWithDuck(proxy);

	return 0;
}
