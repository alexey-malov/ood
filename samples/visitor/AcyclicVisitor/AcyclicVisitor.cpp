// AcyclicVisitor.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <format>
#include <iostream>
#include <memory>
#include <vector>

class VisitorBase
{
public:
	virtual ~VisitorBase() = default;
};

template <typename Visitable>
class Visitor
{
public:
	virtual void Visit(Visitable&) = 0;
};

class Shape
{
public:
	virtual void Accept(VisitorBase& v) const = 0;
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

	void Accept(VisitorBase& visitor) const override
	{
		if (auto* v = dynamic_cast<Visitor<const Rectangle>*>(&visitor))
		{
			v->Visit(*this);
		}
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

	void Accept(VisitorBase& visitor) const override
	{
		if (auto* v = dynamic_cast<Visitor<const Circle>*>(&visitor))
		{
			v->Visit(*this);
		}
	}

private:
	double m_radius;
};

using Shapes = std::vector<std::shared_ptr<Shape>>;

class ShapeGroup : public Shape
{
public:
	void AddShape(const std::shared_ptr<Shape>& shape)
	{
		m_shapes.push_back(shape);
	}

	size_t GetShapeCount() const
	{
		return m_shapes.size();
	}

	std::shared_ptr<Shape> GetShape(size_t index) const
	{
		return m_shapes.at(index);
	}

	void Accept(VisitorBase& visitor) const override
	{
		if (auto* v = dynamic_cast<Visitor<const ShapeGroup>*>(&visitor))
		{
			v->Visit(*this);
		}
	}

private:
	Shapes m_shapes;
};

class StreamOutputVisitor
	: public Visitor<const Rectangle>
	, public Visitor<const Circle>
	, public Visitor<const ShapeGroup>
	, public VisitorBase
{
public:
	StreamOutputVisitor(std::ostream& out)
		: m_out(out)
	{
	}

	void Visit(const Rectangle& rectangle) override
	{
		m_out << std::string(m_indent, ' ') << std::format(R"(<rectangle width="%1%" height="%2%"/>)", rectangle.GetWidth(), rectangle.GetHeight()) << std::endl;
	}

	void Visit(const Circle& circle) override
	{
		m_out << std::string(m_indent, ' ') << std::format(R"(<circle radius="%1%"/>)", circle.GetRadius()) << std::endl;
	}

	void Visit(ShapeGroup const& group) override
	{
		m_out << std::string(m_indent, ' ') << "<group>" << std::endl;
		m_indent += 2;
		for (size_t i = 0; i < group.GetShapeCount(); ++i)
		{
			group.GetShape(i)->Accept(*this);
		}
		m_indent -= 2;
		m_out << std::string(m_indent, ' ') << "</group>" << std::endl;
	}

private:
	std::ostream& m_out;
	size_t m_indent = 0;
};

std::ostream& operator<<(std::ostream& out, const Shape& sh)
{
	StreamOutputVisitor visitor(out);
	sh.Accept(visitor);
	return out;
}

void PrintShapes(const Shapes& shapes, std::ostream& out)
{
	StreamOutputVisitor visitor(out);
	for (auto& shape : shapes)
	{
		shape->Accept(visitor);
	}
}

int main()
{
	auto group = std::make_shared<ShapeGroup>();
	group->AddShape(std::make_shared<Circle>(31));
	group->AddShape(std::make_shared<Rectangle>(15, 16));

	auto group1 = std::make_shared<ShapeGroup>();
	group1->AddShape(std::make_shared<Rectangle>(15, 15));
	group->AddShape(group1);

	Shapes shapes = { std::make_shared<Circle>(42), group, std::make_shared<Rectangle>(30, 20) };
	PrintShapes(shapes, std::cout);
}
