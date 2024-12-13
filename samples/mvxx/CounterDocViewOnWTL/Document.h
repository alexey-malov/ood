#pragma once

class IDocumentListener
{
public:
	virtual void OnDocumentChanged() = 0;

protected:
	~IDocumentListener() = default;
};

class Document
{
public:
	void AddListener(IDocumentListener& listener)
	{
		m_listeners.emplace(&listener);
	}
	void RemoveListener(IDocumentListener& listener)
	{
		m_listeners.erase(&listener);
	}

	int GetCounter() const noexcept { return m_counter; };

	void Increment()
	{
		++m_counter;
		for (auto& listener : m_listeners)
		{
			listener->OnDocumentChanged();
		}
	}

private:
	int m_counter = 0;
	std::unordered_set<IDocumentListener*> m_listeners;
};