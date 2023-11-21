#pragma once
#include <cassert>
#include <memory>

template <typename Value>
class CoW
{
	struct WriteProxy
	{
		explicit WriteProxy(Value* value) noexcept
			: value_ptr_{ value }
		{
		}

		WriteProxy(const WriteProxy&) = delete;
		WriteProxy& operator=(const WriteProxy&) = delete;

		Value& operator*() const& = delete;

		[[nodiscard]] Value& operator*() const&& noexcept
		{
			return *value_ptr_;
		}

		Value* operator->() const& = delete;

		Value* operator->() const&& noexcept
		{
			return value_ptr_;
		}

	private:
		Value* value_ptr_;
	};

public:
	// Конструируем значение по умолчанию.
	CoW()
		: value_(std::make_shared<Value>())
	{
	}

	// Создаём значение за счёт перемещения его из value.
	CoW(Value&& value)
		: value_(std::make_shared<Value>(std::move(value)))
	{
	}

	// Создаём значение из value.
	CoW(const Value& value)
		: value_(std::make_shared<Value>(value))
	{
	}

	// Оператор разыменования служит для чтения значения.
	const Value& operator*() const noexcept
	{
		assert(value_);
		return *value_;
	}

	// Оператор -> служит для чтения полей и вызова константных методов.
	const Value* operator->() const noexcept
	{
		assert(value_);
		return value_.get();
	}

	template <typename ModifierFn>
	void Write(ModifierFn&& modify)
	{
		EnsureUnique();

		std::forward<ModifierFn>(modify)(*value_);
	}

	// Метод Write() нельзя вызвать только у rvalue-ссылок на CoW-объект.
	WriteProxy Write() && = delete;
	[[nodiscard]] WriteProxy Write() &
	{
		EnsureUnique();

		return WriteProxy(value_.get());
	}

	Value& WriteBad()
	{
		EnsureUnique();

		return *value_;
	}

private:
	void EnsureUnique()
	{
		assert(value_);

		if (value_.use_count() > 1)
		{
			// Кроме нас на value_ ссылается кто-то ещё, копируем value_.
			value_ = std::make_shared<Value>(*value_);
		}
	}

	std::shared_ptr<Value> value_;
};
