#include <format>
#include <iostream>
#include <memory>
#include <vector>

using namespace std;

class IShapeVisitor;

class Shape
{
public:
	virtual void Accept(IShapeVisitor& v) const = 0;
	virtual ~Shape() = default;
};

class Rectangle;
class Circle;
class ShapeGroup;

class IShapeVisitor
{
public:
	virtual void Visit(Rectangle const& rectangle) = 0;
	virtual void Visit(Circle const& circle) = 0;
	virtual void Visit(ShapeGroup const& group) = 0;
	virtual ~IShapeVisitor() = default;
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

	void Accept(IShapeVisitor& visitor) const override
	{
		visitor.Visit(*this);
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

	void Accept(IShapeVisitor& visitor) const override
	{
		visitor.Visit(*this);
	}

private:
	double m_radius;
};

typedef vector<shared_ptr<Shape>> Shapes;

class ShapeGroup : public Shape
{
public:
	void AddShape(const shared_ptr<Shape>& shape)
	{
		m_shapes.push_back(shape);
	}

	size_t GetShapeCount() const
	{
		return m_shapes.size();
	}

	shared_ptr<Shape> GetShape(size_t index) const
	{
		return m_shapes.at(index);
	}

	void Accept(IShapeVisitor& v) const override
	{
		v.Visit(*this);
	}

private:
	Shapes m_shapes;
};

class StreamOutputVisitor : public IShapeVisitor
{
public:
	StreamOutputVisitor(ostream& out)
		: m_out(out)
	{
	}

	void Visit(Rectangle const& rectangle) override
	{
		m_out << string(m_indent, ' ')
			  << std::format(R"(<rectangle width="{}" height="{}"/>)",
					 rectangle.GetWidth(), rectangle.GetHeight())
			  << endl;
	}

	void Visit(Circle const& circle) override
	{
		m_out << string(m_indent, ' ')
			  << std::format(R"(<circle radius="{}"/>)", circle.GetRadius()) << endl;
	}

	void Visit(ShapeGroup const& group) override
	{
		m_out << string(m_indent, ' ') << "<group>" << endl;
		m_indent += 2;
		for (size_t i = 0; i < group.GetShapeCount(); ++i)
		{
			group.GetShape(i)->Accept(*this);
		}
		m_indent -= 2;
		m_out << string(m_indent, ' ') << "</group>" << endl;
	}

private:
	ostream& m_out;
	size_t m_indent = 0;
};

ostream& operator<<(ostream& out, const Shape& sh)
{
	StreamOutputVisitor visitor(out);
	sh.Accept(visitor);
	return out;
}

void PrintShapes(const Shapes& shapes, ostream& out)
{
	StreamOutputVisitor visitor(out);
	for (auto& shape : shapes)
	{
		shape->Accept(visitor);
	}
}

int main()
{
	auto group = make_shared<ShapeGroup>();
	group->AddShape(make_shared<Circle>(31));
	group->AddShape(make_shared<Rectangle>(15, 16));

	auto group1 = make_shared<ShapeGroup>();
	group1->AddShape(make_shared<Rectangle>(15, 15));
	group->AddShape(group1);

	Shapes shapes = { make_shared<Circle>(42), group, make_shared<Rectangle>(30, 20) };
	PrintShapes(shapes, cout);
	return 0;
}
