#include <format>
#include <iostream>
#include <memory>
#include <vector>

using namespace std;

class IShapeVisitor;

class CShape
{
public:
	virtual void Accept(IShapeVisitor& v) const = 0;
	virtual ~CShape() = default;
};

class CRectangle;
class CCircle;
class CShapeGroup;

class IShapeVisitor
{
public:
	virtual void Visit(CRectangle const& rectangle) = 0;
	virtual void Visit(CCircle const& circle) = 0;
	virtual void Visit(CShapeGroup const& group) = 0;
	virtual ~IShapeVisitor() = default;
};

class CRectangle : public CShape
{
public:
	CRectangle(double w, double h)
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

class CCircle : public CShape
{
public:
	CCircle(double r)
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

typedef vector<shared_ptr<CShape>> Shapes;

class CShapeGroup : public CShape
{
public:
	void AddShape(const shared_ptr<CShape>& shape)
	{
		m_shapes.push_back(shape);
	}

	size_t GetShapeCount() const
	{
		return m_shapes.size();
	}

	shared_ptr<CShape> GetShape(size_t index) const
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

class CStreamOutputVisitor : public IShapeVisitor
{
public:
	CStreamOutputVisitor(ostream& out)
		: m_out(out)
	{
	}

	void Visit(CRectangle const& rectangle) override
	{
		m_out << string(m_indent, ' ')
			  << std::format(R"(<rectangle width="%1%" height="%2%"/>)",
					 rectangle.GetWidth(), rectangle.GetHeight())
			  << endl;
	}

	void Visit(CCircle const& circle) override
	{
		m_out << string(m_indent, ' ')
			  << std::format(R"(<circle radius="%1%"/>)", circle.GetRadius()) << endl;
	}

	void Visit(CShapeGroup const& group) override
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

ostream& operator<<(ostream& out, const CShape& sh)
{
	CStreamOutputVisitor visitor(out);
	sh.Accept(visitor);
	return out;
}

void PrintShapes(const Shapes& shapes, ostream& out)
{
	CStreamOutputVisitor visitor(out);
	for (auto& shape : shapes)
	{
		shape->Accept(visitor);
	}
}

int main()
{
	auto group = make_shared<CShapeGroup>();
	group->AddShape(make_shared<CCircle>(31));
	group->AddShape(make_shared<CRectangle>(15, 16));

	auto group1 = make_shared<CShapeGroup>();
	group1->AddShape(make_shared<CRectangle>(15, 15));
	group->AddShape(group1);

	Shapes shapes = { make_shared<CCircle>(42), group, make_shared<CRectangle>(30, 20) };
	PrintShapes(shapes, cout);
	return 0;
}
