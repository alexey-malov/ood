#pragma once
#include "CommonTypes.h"
#include "Canvas.h"
#include <memory>
#include <numeric>

namespace version1
{

class CStyle
{
public:
	bool IsEnabled()const;
	void Enable(bool enable);

	RGBAColor GetColor()const;
	void SetColor(RGBAColor color);
};

class CShape
{
public:
	RectD GetFrame();
	void SetFrame(const RectD & rect);

	CStyle GetLineStyle()const;
	void SetLineStyle(const CStyle& style);

	CStyle GetFillStyle()const;
	void SetFillStyle(const CStyle& style);

	virtual void Draw(ICanvas & canvas) const = 0;

	virtual ~CShape() = default;
};

class CRectangle : public CShape
{
public:
	void Draw(ICanvas & canvas) const final;
};

class CEllipse : public CShape
{
public:
	void Draw(ICanvas & canvas) const final;
};

class CTriangle : public CShape
{
public:
	void Draw(ICanvas & canvas) const final;
};

class CSlide
{
public:
	double GetWidth()const;
	double GetHeight()const;

	size_t GetShapesCount()const;
	std::shared_ptr<CShape> GetShapeAtIndex(size_t index);
	void InsertShape(const std::shared_ptr<CShape> & shape, size_t position = std::numeric_limits<size_t>::max());
	void RemoveShapeAtIndex(size_t index);

	RGBAColor GetBackgroundColor()const;
	void SetBackgroundColor(RGBAColor color);

	void Draw(const ICanvas & canvas);
};


}