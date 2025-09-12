#include <algorithm>
#include <concepts>
#include <iostream>
#include <iterator>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

using namespace std;

class Shape
{
public:
	virtual unique_ptr<Shape> Clone() const = 0;
	virtual ~Shape() = default;
};

class Rectangle : public Shape
{
public:
	Rectangle(double w, double h)
		: m_width(w)
		, m_height(h)
	{
	}
	double GetWidth() const
	{
		return m_width;
	}

	double GetHeight() const
	{
		return m_height;
	}

	unique_ptr<Shape> Clone() const override
	{
		return make_unique<Rectangle>(*this);
	}

private:
	double m_width, m_height;
};

class Circle : public Shape
{
public:
	Circle(double r)
		: m_radius(r)
	{
	}
	double GetRadius() const
	{
		return m_radius;
	}

	unique_ptr<Shape> Clone() const override
	{
		return make_unique<Circle>(*this);
	}

private:
	double m_radius;
};

typedef vector<shared_ptr<Shape>> Shapes;

class ShapeRegistry
{
public:
	void RegisterShapePrototype(const string& id, const Shape& prototype)
	{
		m_prototypes.emplace(id, prototype.Clone());
	}
	unique_ptr<Shape> CreateShape(const string& id)
	{
		return m_prototypes.at(id)->Clone();
	}

private:
	unordered_map<string, unique_ptr<Shape>> m_prototypes;
};

template <typename Impl, typename CloneInterface, typename Base = CloneInterface>
class PrototypeImpl : public Base
{
public:
	// Используем конструктор базового класса
	using Base::Base;

	unique_ptr<CloneInterface> Clone() const override
	{
		return make_unique<Impl>(*static_cast<Impl const*>(this));
	}
};

class Point : public PrototypeImpl<Point, Shape>
{
public:
	Point() = default;
	Point(double x, double y)
		: m_x(x)
		, m_y(y)
	{
	}

private:
	double m_x = 0.0;
	double m_y = 0.0;
};

class ShapeGroup : public PrototypeImpl<ShapeGroup, Shape>
{
public:
	ShapeGroup() = default;
	ShapeGroup(const ShapeGroup& other)
	{
		m_shapes.reserve(other.m_shapes.size());
		for (auto& shape : other.m_shapes)
		{
			m_shapes.push_back(shape->Clone());
		}
	}
	ShapeGroup& operator=(const ShapeGroup& other)
	{
		if (this != &other) // Реализуем присваивание через конструктор копирования
		{
			m_shapes = std::move(ShapeGroup(other).m_shapes);
		}
		return *this;
	}

	void AddShape(unique_ptr<Shape>&& shape)
	{
		m_shapes.push_back(move(shape));
	}

	size_t GetShapeCount() const
	{
		return m_shapes.size();
	}

	shared_ptr<Shape> GetShape(size_t index) const
	{
		return m_shapes.at(index);
	}

private:
	Shapes m_shapes;
};

class IPerson
{
public:
	virtual ~IPerson() = default;
	virtual unique_ptr<IPerson> Clone() const = 0;
	virtual string GetName() const = 0;
	virtual void SpendDay() = 0;
};

namespace alternative1
{

class Person : public PrototypeImpl<Person, IPerson>
{
public:
	explicit Person(std::string name)
		: m_name(std::move(name))
	{
	}

	string GetName() const override
	{
		return m_name;
	}

	void SpendDay() override
	{
		// Do nothing
	}

private:
	std::string m_name;
};

class Worker : public PrototypeImpl<Worker, IPerson, Person>
{
public:
	explicit Worker(std::string name, std::string speciality)
		: PrototypeImpl(std::move(name))
		, m_speciality(std::move(speciality))
	{
	}
	void SpendDay() override
	{
		std::cout << GetName() << " works as " << m_speciality << std::endl;
	}

private:
	std::string m_speciality;
};
} // namespace alternative1

namespace alternative2
{

template <typename Base>
class PersonImpl : public Base
{
public:
	PersonImpl(std::string name)
		: m_name(std::move(name))
	{
	}

	std::string GetName() const final
	{
		return m_name;
	}

private:
	std::string m_name;
};

class Person final
	: public PrototypeImpl<Person, IPerson, PersonImpl<IPerson>>
{
public:
	explicit Person(std::string name)
		: PrototypeImpl(std::move(name))
	{
	}

	void SpendDay() override
	{
		// Do nothing
	}
};

class Worker final
	: public PrototypeImpl<Worker, IPerson, PersonImpl<IPerson>>
{
public:
	explicit Worker(std::string name, std::string speciality)
		: PrototypeImpl(std::move(name))
		, m_speciality(std::move(speciality))
	{
	}
	void SpendDay() override
	{
		std::cout << GetName() << " works as "
				  << m_speciality << std::endl;
	}

private:
	std::string m_speciality;
};

} // namespace alternative2

namespace alternative3
{

inline namespace prototype
{
// Проверяем, что T — полиморфный тип с виртуальным деструктором
template <typename T>
concept Polymorphic = std::has_virtual_destructor_v<T>;

// Проверяем, что T имеет метод Clone, возвращающий unique_ptr<T>
template <typename CI>
concept CloneInterfaceLike = Polymorphic<CI> && requires(CI const& x) {
	{ x.Clone() } -> std::same_as<std::unique_ptr<CI>>;
};

// Проверяем, что Impl наследует Base, а Base — CloneInterface
// *** ВАЖНО: без derived_from<Impl, Base> — MSVC не любит неполные типы ***

// 
template <typename CI, typename Base>
concept PrototypeBaseOk = CloneInterfaceLike<CI> && std::derived_from<Base, CI>;

template <typename Impl, typename CloneInterface, typename Base = CloneInterface>
	requires PrototypeBaseOk<CloneInterface, Base>
class PrototypeImpl : public Base
{
public:
	// Перенаправляем параметры в конструктор Base
	template <typename... Args>
	explicit PrototypeImpl(Args&&... args)
		: Base(std::forward<Args>(args)...)
	{
	}

	[[nodiscard]] std::unique_ptr<CloneInterface> Clone() const override
	{
		// Безопасная точка для «жёстких» проверок, если очень хочется:
		static_assert(std::is_base_of_v<Base, Impl>);
		static_assert(std::is_copy_constructible_v<Impl>);
		return std::make_unique<Impl>(static_cast<Impl const&>(*this));
	}
};
} // namespace prototype

inline namespace person_impl
{

// Проверяем, что Base имеет метод GetName, возвращающий std::string
template <typename Base>
concept NameInterfaceLike = Polymorphic<Base> && requires(Base const& b) {
	{ b.GetName() } -> std::same_as<std::string>;
};

// Проверяем, что S можно использовать для инициализации std::string
template <typename S>
concept StringLike = requires(S const& s) { std::string{ s }; };

// Реализация PersonImpl, добавляющая поле name и реализующая GetName
template <typename Base>
	requires NameInterfaceLike<Base> // Base должен иметь GetName
class PersonImpl : public Base
{
public:
	using Base::Base;

	// Конструктор от любого StringLike
	template <StringLike S>
	explicit PersonImpl(S&& name) noexcept(std::is_nothrow_constructible_v<std::string, S>)
		: Base()
		, m_name(std::forward<S>(name))
	{
	}

	std::string GetName() const final { return m_name; }

private:
	std::string m_name;
};
} // namespace person_impl

class Person; // forward

using PersonBase = PrototypeImpl<Person, IPerson, PersonImpl<IPerson>>;

class Person final : public PersonBase
{
public:
	using PersonBase::PersonBase; // корректно наследуем ctors
	void SpendDay() override { /* ничего */ }
};

class Worker; // forward

using WorkerBase = PrototypeImpl<Worker, IPerson, PersonImpl<IPerson>>;

class Worker final : public WorkerBase
{
public:
	Worker(std::string name, std::string speciality)
		: WorkerBase(std::move(name))
		, m_speciality(std::move(speciality))
	{
	}

	void SpendDay() override
	{
		std::cout << GetName() << " works as " << m_speciality << std::endl;
	}

private:
	std::string m_speciality;
};

} // namespace alternative3

int main()
{
	{
		ShapeRegistry registry;
		registry.RegisterShapePrototype("circle", Circle(42));
		registry.RegisterShapePrototype("rectangle", Rectangle(40, 30));
		registry.RegisterShapePrototype("point", Point(40, 30));
		ShapeGroup group;
		group.AddShape(make_unique<Circle>(10));
		group.AddShape(make_unique<Rectangle>(32, 32));
		registry.RegisterShapePrototype("group", group);

		auto someShape = registry.CreateShape("group");
	}

	{
		using namespace alternative1;
		unique_ptr<IPerson> person = make_unique<Worker>("Mario", "Plumber");
		person->SpendDay();
		person->Clone()->SpendDay();
	}
	{
		using namespace alternative2;
		unique_ptr<IPerson> person = make_unique<Worker>("Mario", "Plumber");
		person->SpendDay();
		person->Clone()->SpendDay();
	}
	{
		using namespace alternative3;
		unique_ptr<IPerson> person = make_unique<Worker>("Mario", "Plumber");
		person->SpendDay();
		person->Clone()->SpendDay();
	}
}
