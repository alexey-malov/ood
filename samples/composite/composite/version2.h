#pragma once

#include "Canvas.h"

namespace version2
{

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

}
