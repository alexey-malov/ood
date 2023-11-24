#pragma once
#include <iosfwd>
#include <string>

#include "geom.h"

class Image
{
public:
	/**
     * Конструирует изображение заданного размера. Если размеры не являются положительными,
     * выбрасывает исключение std::out_of_range.
     */
	explicit Image(Size size, char color = ' ');

	// Возвращает размер изображения в пикселях.
	Size GetSize() const noexcept;

	/**
     * Возвращает «цвет» пикселя в указанных координатах.Если координаты выходят за пределы
     * изображения, возвращает «пробел».
     */
	char GetPixel(Point p) const noexcept;

	/**
     * Задаёт «цвет» пикселя в указанных координатах. Если координаты выходят за пределы изображения
     * действие игнорируется.
     */
	void SetPixel(Point p, char color);

private:
};

/**
 * Выводит в поток out изображение в виде символов.
 */
void Print(const Image& img, std::ostream& out);

/**
 * Загружает изображение из pixels. Линии изображения разделяются символами \n.
 * Размеры картинки определяются по количеству переводов строки и самой длинной линии.
 */
Image LoadImage(const std::string& pixels);
