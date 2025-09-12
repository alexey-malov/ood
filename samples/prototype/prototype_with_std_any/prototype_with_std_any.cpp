// prototype_with_std_any.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <any>
#include <iostream>
#include <memory>
#include <numbers>
#include <vector>

#define CATCH_CONFIG_ENABLE_BENCHMARKING
#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>

class IShape
{
public:
	virtual ~IShape() = default;

	virtual void Draw(std::ostream& out) const = 0;
	virtual double GetArea() const = 0;
};

class Circle : public IShape
{
public:
	explicit Circle(double radius) noexcept
		: m_radius(radius)
	{
	}

	double GetRadius() const noexcept
	{
		return m_radius;
	}

	void Draw(std::ostream& out) const override
	{
		out << "Drawing a circle with radius " << m_radius << '\n';
	}
	double GetArea() const final
	{
		return std::numbers::pi * m_radius * m_radius;
	}

private:
	double m_radius;
};

class Rect : public IShape
{
public:
	explicit Rect(double width, double height) noexcept
		: m_width(width)
		, m_height(height)
	{
	}

	double GetWidth() const noexcept
	{
		return m_width;
	}

	double GetHeight() const noexcept
	{
		return m_height;
	}

	void Draw(std::ostream& out) const override
	{
		std::cout << "Drawing a rectangle with width "
				  << m_width << " and height " << m_height << '\n';
	}
	double GetArea() const final
	{
		return m_width * m_height;
	}

private:
	double m_width;
	double m_height;
};

namespace wont_work
{
class Shape
{
public:
	template <std::derived_from<IShape> Impl>
	Shape(Impl&& impl)
		: m_impl(std::forward<Impl>(impl))
	{
	}

	void Draw(std::ostream& out) const
	{
		GetImpl().Draw(out);
	}

	double GetArea() const
	{
		return GetImpl().GetArea();
	}

private:
	const IShape& GetImpl() const
	{
		return std::any_cast<const IShape&>(m_impl);
	}
	IShape& GetImpl()
	{
		return std::any_cast<IShape&>(m_impl);
	}
	std::any m_impl;
};
} // namespace wont_work

namespace naive
{
class Shape
{
public:
	Shape() = default;

	template <typename Impl>
		requires(std::derived_from<std::remove_cvref_t<Impl>, IShape>
					&& !std::is_same_v<std::remove_cvref_t<Impl>, Shape>)
	Shape(Impl&& impl)
		: m_impl(std::forward<Impl>(impl))
		, m_getter([](const std::any& a) -> const IShape& {
			return std::any_cast<const Impl&>(a);
		})
	{
	}

	void Draw(std::ostream& out) const
	{
		GetImpl().Draw(out);
	}

	double GetArea() const
	{
		return GetImpl().GetArea();
	}

	template <typename Type, typename... Args>
		requires(std::derived_from<std::remove_cvref_t<Type>, IShape>)
	const Type& As() const
	{
		return dynamic_cast<const std::remove_cvref_t<Type>&>(GetImpl());
	}

	template <typename Type, typename... Args>
		requires(std::derived_from<std::remove_cvref_t<Type>, IShape>)
	Type& As()
	{
		return dynamic_cast<std::remove_cvref_t<Type>&>(GetImpl());
	}

private:
	const IShape& GetImpl() const
	{
		return m_getter(m_impl);
	}
	IShape& GetImpl()
	{
		return const_cast<IShape&>(m_getter(m_impl));
	}

	std::any m_impl;
	const IShape& (*m_getter)(const std::any&) = nullptr;
};

} // namespace works

namespace optimized
{
class Shape
{
public:
	Shape() = default;

	// Конструктор от любого наследника IShape (не от самого Shape)
	template <typename Impl>
		requires(std::derived_from<std::remove_cvref_t<Impl>, IShape>
			&& !std::same_as<std::remove_cvref_t<Impl>, Shape>)
	explicit Shape(Impl&& impl)
	{
		using T = std::remove_cvref_t<Impl>; // снимаем cvref → внутрь any кладём НЕконстантный T
		m_impl.emplace<T>(std::forward<Impl>(impl)); // value-semantics
		// Ребиндер знает реальный T и умеет вернуть IShape* из std::any
		m_rebind = [](std::any& a) -> IShape* { return &std::any_cast<T&>(a); };
		Rebind(); // кешируем адрес внутри any
	}

	// Специальные члены — просто переносим/копируем state и ребиндим указатель
	Shape(const Shape& other)
		: m_impl(other.m_impl)
		, m_rebind(other.m_rebind)
	{
		Rebind();
	}

	Shape(Shape&& other) noexcept
		: m_impl(std::move(other.m_impl))
		, m_rebind(other.m_rebind)
	{
		Rebind();
		other.Reset();
	}

	Shape& operator=(const Shape& rhs)
	{
		if (this != &rhs)
		{
			m_impl = rhs.m_impl;
			m_rebind = rhs.m_rebind;
			Rebind();
		}
		return *this;
	}

	Shape& operator=(Shape&& rhs) noexcept
	{
		if (this != &rhs)
		{
			m_impl = std::move(rhs.m_impl);
			m_rebind = rhs.m_rebind;
			Rebind();
			rhs.Reset();
		}
		return *this;
	}

	// Публичный API
	void Draw(std::ostream& out) const
	{
		GetInterface().Draw(out); // не нужен const_cast: неявно приводим IShape& → const IShape&
	}

	double GetArea() const
	{
		return GetInterface().GetArea();
	}

	template <typename Type>
		requires(std::derived_from<std::remove_cvref_t<Type>, IShape>)
	const Type& As() const
	{
		return dynamic_cast<const std::remove_cvref_t<Type>&>(GetInterface());
	}

	template <typename Type>
		requires(std::derived_from<std::remove_cvref_t<Type>, IShape>)
	Type& As()
	{
		return dynamic_cast<std::remove_cvref_t<Type>&>(GetMutableInterface());
	}

	bool HasValue() const noexcept { return m_iface != nullptr; }

	const IShape& GetInterface() const
	{
		assert(m_iface && "Shape is empty");
		return *m_iface; // IShape& → const IShape& (без const_cast)
	}

	IShape& GetMutableInterface()
	{
		assert(m_iface && "Shape is empty");
		return *m_iface;
	}

private:
	void Rebind() noexcept
	{
		m_iface = (m_rebind && m_impl.has_value()) ? m_rebind(m_impl) : nullptr;
	}

	void Reset() noexcept
	{
		m_impl.reset();
		m_rebind = nullptr;
		m_iface = nullptr;
	}

private:
	std::any m_impl;
	// type-erased «ребиндер»: знает реальный T и возвращает IShape* из any
	IShape* (*m_rebind)(std::any&) = nullptr;
	// кеш «горячего» указателя
	IShape* m_iface = nullptr;
};

} // namespace optimized

TEST_CASE("Wrong shape tests")
{
	using namespace wont_work;

	Shape circle = Circle{ 5.0 };
	CHECK_THROWS_AS(circle.GetArea(), std::bad_any_cast);

	Shape rectangle = Rect{ 4.0, 6.0 };
	CHECK_THROWS_AS(rectangle.GetArea(), std::bad_any_cast);
}

TEST_CASE("Shape tests")
{
	using namespace naive;
	Shape circle = Circle{ 5.0 };
	CHECK(circle.GetArea() == Approx(78.53981633974483));
	CHECK(circle.As<Circle>().GetRadius() == Approx(5.0));

	Shape rectangle = Rect{ 4.0, 6.0 };
	CHECK(rectangle.GetArea() == Approx(24.0));
	CHECK(rectangle.As<Rect>().GetWidth() == Approx(4.0));

	SECTION("Reserved construction")
	{
		constexpr size_t count = 100'000;
		SECTION("Shape")
		{
			std::vector<Shape> shapes;
			shapes.reserve(count);
			BENCHMARK("Construction with std::any")
			{
				shapes.clear();
				for (size_t i = 0; i < count; ++i)
				{
					shapes.emplace_back(Circle{ static_cast<double>(i) });
				}
				return shapes.size();
			};
		}

		SECTION("IShape")
		{
			std::vector<std::unique_ptr<IShape>> shapePtrs;
			shapePtrs.reserve(count);
			BENCHMARK("Construction in heap")
			{
				shapePtrs.clear();
				for (size_t i = 0; i < count; ++i)
				{
					shapePtrs.emplace_back(std::make_unique<Circle>(static_cast<double>(i)));
				}
				return shapePtrs.size();
			};
		}
	}

	SECTION("Unreserved construction")
	{
		constexpr size_t count = 100'000;
		SECTION("Shape")
		{
			std::vector<Shape> shapes;
			BENCHMARK("Construction with std::any")
			{
				shapes.clear();
				for (size_t i = 0; i < count; ++i)
				{
					shapes.emplace_back(Circle{ static_cast<double>(i) });
				}
				return shapes.size();
			};
		}

		SECTION("IShape")
		{
			std::vector<std::unique_ptr<IShape>> shapePtrs;
			BENCHMARK("Construction in heap")
			{
				shapePtrs.clear();
				for (size_t i = 0; i < count; ++i)
				{
					shapePtrs.emplace_back(std::make_unique<Circle>(static_cast<double>(i)));
				}
				return shapePtrs.size();
			};
		}
	}
}

TEST_CASE("Area computation benchmarks (sum over container)")
{
	constexpr std::size_t count = 100'000;

	// --- Подготовка данных (не входит в измерение) ---
	std::vector<std::unique_ptr<IShape>> ptrs;
	ptrs.reserve(count);

	std::vector<naive::Shape> values;
	values.reserve(count);

	// Наполним контейнеры одинаковым набором фигур (50% Circle, 50% Rect)
	for (std::size_t i = 0; i < count; ++i)
	{
		if ((i & 1u) == 0u)
		{
			double r = static_cast<double>(i % 100 + 1); // радиусы 1..100 по кругу
			ptrs.emplace_back(std::make_unique<Circle>(r));
			values.emplace_back(Circle{ r });
		}
		else
		{
			double w = static_cast<double>(i % 50 + 1); // ширины 1..50
			double h = static_cast<double>((i * 3) % 50 + 1); // высоты 1..50
			ptrs.emplace_back(std::make_unique<Rect>(w, h));
			values.emplace_back(Rect{ w, h });
		}
	}

	// Небольшая проверка корректности вне бенчмарков
	{
		double s1 = 0.0, s2 = 0.0;
		for (auto const& p : ptrs)
			s1 += p->GetArea();
		for (auto const& v : values)
			s2 += v.GetArea();
		REQUIRE(s1 == Approx(s2));
	}

	// --- Измерения ---

	BENCHMARK_ADVANCED("sum GetArea - unique_ptr<IShape>")
	(Catch::Benchmark::Chronometer meter)
	{
		meter.measure([&] {
			double sum = 0.0;
			for (auto const& p : ptrs)
			{
				sum += p->GetArea();
			}
			return sum; // возврат значения мешает агрессивному выпиливанию оптимизатором
		});
	};

	BENCHMARK_ADVANCED("sum GetArea - naive::Shape (std::any)")
	(Catch::Benchmark::Chronometer meter)
	{
		meter.measure([&] {
			double sum = 0.0;
			for (auto const& v : values)
			{
				sum += v.GetArea();
			}
			return sum;
		});
	};
}

TEST_CASE("Micro-benchmark: sum of areas (any: naive vs optimized vs unique_ptr)")
{
	using std::make_unique;

	constexpr std::size_t count = 100'000;

	// Подготовка данных (НЕ входит в измерения)
	std::vector<naive::Shape> v_naive;
	v_naive.reserve(count);
	std::vector<optimized::Shape> v_cached;
	v_cached.reserve(count);
	std::vector<std::unique_ptr<IShape>> v_ptrs;
	v_ptrs.reserve(count);

	for (std::size_t i = 0; i < count; ++i)
	{
		if ((i & 1u) == 0u)
		{
			double r = static_cast<double>(i % 100 + 1);
			v_naive.emplace_back(Circle{ r });
			v_cached.emplace_back(Circle{ r });
			v_ptrs.emplace_back(make_unique<Circle>(r));
		}
		else
		{
			double w = static_cast<double>(i % 50 + 1);
			double h = static_cast<double>((i * 3) % 50 + 1);
			v_naive.emplace_back(Rect{ w, h });
			v_cached.emplace_back(Rect{ w, h });
			v_ptrs.emplace_back(make_unique<Rect>(w, h));
		}
	}

	// Вне бенчей — быстрая проверка эквивалентности результатов
	auto sumNaive = [&] { double s=0; for (auto const& x: v_naive)  s += x.GetArea(); return s; };
	auto sumOptimized = [&] { double s=0; for (auto const& x: v_cached) s += x.GetArea(); return s; };
	auto sumPtrs = [&] { double s=0; for (auto const& p: v_ptrs)   s += p->GetArea(); return s; };
	REQUIRE(sumNaive() == Approx(sumOptimized()));
	REQUIRE(sumOptimized() == Approx(sumPtrs()));

	BENCHMARK_ADVANCED("sum GetArea - any (naive any_cast per call)")
	(Catch::Benchmark::Chronometer meter)
	{
		meter.measure([&] {
			double s = 0.0;
			for (auto const& x : v_naive)
				s += x.GetArea();
			return s;
		});
	};

	BENCHMARK_ADVANCED("sum GetArea - any (cached IShape*)")
	(Catch::Benchmark::Chronometer meter)
	{
		meter.measure([&] {
			double s = 0.0;
			for (auto const& x : v_cached)
				s += x.GetArea();
			return s;
		});
	};

	BENCHMARK_ADVANCED("sum GetArea - unique_ptr<IShape>")
	(Catch::Benchmark::Chronometer meter)
	{
		meter.measure([&] {
			double s = 0.0;
			for (auto const& p : v_ptrs)
				s += p->GetArea();
			return s;
		});
	};
}