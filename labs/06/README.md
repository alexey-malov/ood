
# **Лабораторная работа №6. Паттерн «Адаптер»**

- [**Лабораторная работа №6. Паттерн «Адаптер»**](#лабораторная-работа-6-паттерн-адаптер)
  - [**Обязательные задания**](#обязательные-задания)
    - [**Задание 1 – Адаптер объекта – 100 баллов**](#задание-1--адаптер-объекта--100-баллов)
      - [***Бонус – 40 баллов за unit-тестирование адаптера***](#бонус--40-баллов-за-unit-тестирование-адаптера)
      - [***Бонус – 20 баллов за UML-диаграмму классов***](#бонус--20-баллов-за-uml-диаграмму-классов)
  - [**Дополнительные задания**](#дополнительные-задания)
    - [**Задание 2 – Адаптер класса – 30 баллов**](#задание-2--адаптер-класса--30-баллов)
      - [**Бонус – 40 баллов за unit-тестирование адаптера**](#бонус--40-баллов-за-unit-тестирование-адаптера-1)
      - [***Бонус – 10 баллов за UML-диаграмму классов***](#бонус--10-баллов-за-uml-диаграмму-классов)
    - [**Задание 3 – Поддержка цветных изображений – 60 баллов**](#задание-3--поддержка-цветных-изображений--60-баллов)
      - [***Бонус – 30 баллов за unit-тестирование адаптера***](#бонус--30-баллов-за-unit-тестирование-адаптера)

На оценку «удовлетворительно» необходимо набрать 100 баллов.

На оценку «хорошо» необходимо набрать 200 баллов.

На оценку «отлично» необходимо набрать 300 баллов.

**Дополнительные задания принимаются только после выполнения обязательных заданий.**

## **Обязательные задания**

### **Задание 1 – Адаптер объекта – 100 баллов**

IT компания разработала приложение позволяющее рисовать треугольники и прямоугольники. В процессе разработки, команда использовала библиотеку **graphics_lib**, предоставляющую API для рисования простейших графических примитивов.

// Пространство имен графической библиотеки (недоступно для изменения)

```cpp
namespace graphics_lib
{
// Холст для рисования
class ICanvas
{
public:
  // Ставит "перо" в точку x, y
  virtual void MoveTo(int x, int y) = 0;
  // Рисует линию с текущей позиции, передвигая перо в точку x,y
  virtual void LineTo(int x, int y) = 0;
  virtual ~ICanvas() = default;
};
// Реализация холста для рисования
class CCanvas : public ICanvas
{
public:
  void MoveTo(int x, int y) override
  {
    cout << "MoveTo (" << x << ", " << y << ")" << endl;
  }
  void LineTo(int x, int y) override
  {
    cout << "LineTo (" << x << ", " << y << ")" << endl;
  }
};
}
```

Для рисования более сложных фигур команда решила использовать библиотеку **shape_drawing_lib**, позволяющую рисовать объекты, используя API библиотеки **graphics_lib**.

Код библиотеки представлен ниже.

```cpp
// Пространство имен библиотеки для рисования фигур (использует graphics\_lib)
// Код библиотеки недоступен для изменения
namespace shape_drawing_lib
{
struct Point
{
  int x;
  int y;
};
// Интерфейс объектов, которые могут быть нарисованы на холсте из graphics\_lib
class ICanvasDrawable
{
public:
  virtual void Draw(graphics_lib::ICanvas & canvas)const = 0;
  virtual ~ICanvasDrawable() = default;
};
class CTriangle : public ICanvasDrawable
{
public:
  CTriangle(const Point & p1, const Point & p2, const Point & p3)
  {
    // TODO: написать код конструктора
  }
  void Draw(graphics_lib::ICanvas & canvas)const override
  {
    // TODO: написать код рисования треугольника на холсте
  }
private:
  // TODO: дописать приватную часть
};
class CRectangle : public ICanvasDrawable
{
public:
  CRectangle(const Point & leftTop, int width, int height)
  {
    // TODO: написать код конструктора
  }
  void Draw(graphics_lib::ICanvas & canvas)const override
  {
    // TODO: написать код рисования прямоугольника на холсте
  }
private:
  // TODO: дописать приватную часть
};
// Художник, способный рисовать ICanvasDrawable-объекты на ICanvas
class CCanvasPainter
{
public:
  CCanvasPainter(graphics_lib::ICanvas & canvas)
  {
    // TODO: дописать конструктор класса
  }
  void Draw(const ICanvasDrawable & drawable)
  {
    // TODO: дописать код рисования ICanvasDrawable на canvas, переданном в конструктор
  }
private:
  // TODO: дописать приватную часть
};
}
Для рисования приложение использует вспомогательный класс CCanvasPainter:
// Пространство имен приложения (доступно для модификации)
namespace app
{
void PaintPicture(shape_drawing_lib::CCanvasPainter & painter)
{
  using namespace shape_drawing_lib;
  CTriangle triangle({ 10, 15 }, { 100, 200 }, { 150, 250 });
  CRectangle rectangle({ 30, 40 }, 18, 24);
  // TODO: нарисовать прямоугольник и треугольник при помощи painter
}
void PaintPictureOnCanvas()
{
  graphics_lib::CCanvas simpleCanvas;
  shape_drawing_lib::CCanvasPainter painter(simpleCanvas);
  PaintPicture(painter);
}
}
int main()
{
  app::PaintPictureOnCanvas();
  return 0;
}
Вскоре вышла графическая библиотека **modern_graphics_lib**, позволяющая рисовать графические примитивы со более высоким качеством.
// Пространство имен современной графической библиотеки (недоступно для изменения)
namespace modern_graphics_lib
{
class CPoint
{
public:
  CPoint(int x, int y) :x(x), y(y) {}
  int x;
  int y;
};

// Класс для современного рисования графики
class CModernGraphicsRenderer
{
public:
  CModernGraphicsRenderer(ostream & strm) : m_out(strm)
  {
  }
  ~CModernGraphicsRenderer()
  {
    if (m_drawing) // Завершаем рисование, если оно было начато
    {
        EndDraw();
    }
  }
  // Этот метод должен быть вызван в начале рисования
  void BeginDraw()
  {
    if (m_drawing)
    {
        throw logic_error("Drawing has already begun");
    }
    m_out << "<draw>" << endl;
    m_drawing = true;
  }
  // Выполняет рисование линии
  void DrawLine(const CPoint & start, const CPoint & end)
  {
    if (!m_drawing)
    {
        throw logic_error("DrawLine is allowed between BeginDraw()/EndDraw() only");
    }
    m_out << boost::format(R"(  <line fromX="%1%" fromY="%2" toX="%3%" toY="%4%"/>)")
          << endl;
  }
  // Этот метод должен быть вызван в конце рисования
  void EndDraw()
  {
    if (!m_drawing)
    {
        throw logic_error("Drawing has not been started");
    }
    m_out << "</draw>" << endl;
    m_drawing = false;
  }
private:
  ostream & m_out;
  bool m_drawing = false;
};
}
```

Использование новой графической библиотеки вместо старой осложняется следующими факторами:

- Используемая библиотека shape_drawing_lib основана на библиотеке graphics_lib, а не modern_graphics_lib
- Разработчики библиотеки shape_drawing_lib пока не заинтересованы в поддержке библиотеки modern_graphics_lib по следующим причинам:
  - И shape_drawing_lib, и graphics_lib являются кроссплатформенными, а modern_graphics_lib – нет
  - У них есть свои планы развития своей библиотеки
- Библиотека modern_graphics_lib требует для своей работы ОС, начиная с Windows 7, в то время как приложение все еще должно поддерживать и более старые операционные системы: Windows XP и Windows Vista, которые прекрасно поддерживаются graphics_lib

Принимая во внимание важность качественного отображения фигур, а также распространенность различных версий Windows, команда приняла решение использовать modern_graphics_lib на современных ОС, а на XP и Vista – graphics_lib.

С использованием адаптера объектов реализуйте данный функционал, не изменяя код ни одной из библиотек, а также код функции PaintPicture.

```cpp
// Пространство имен приложения (доступно для модификации)
namespace app
{
void PaintPicture(shape_drawing_lib::CCanvasPainter & painter);
void PaintPictureOnCanvas();
void PaintPictureOnModernGraphicsRenderer()
{
  modern_graphics_lib::CModernGraphicsRenderer renderer(cout);
  (void)&renderer; // устраняем предупреждение о неиспользуемой переменной
  // TODO: при помощи существующей функции PaintPicture() нарисовать
  // картину на renderer
  // Подсказка: используйте паттерн "Адаптер"
}
}
int main()
{
  cout << "Should we use new API (y)?";
  string userInput;
  if (getline(cin, userInput) && (userInput == "y" || userInput == "Y"))
  {
    app::PaintPictureOnModernGraphicsRenderer();
  }
  else
  {
    app::PaintPictureOnCanvas();
  }
  return 0;
}
```

#### ***Бонус – 40 баллов за unit-тестирование адаптера***

Бонус начисляется за покрытие функционала разработанного адаптера объекта автоматическими тестами

#### ***Бонус – 20 баллов за UML-диаграмму классов***

Нарисуйте UML-диаграмму классов системы, выделите на ней адаптер.

## **Дополнительные задания**

### **Задание 2 – Адаптер класса – 30 баллов**

На основе программы из задания «Адаптер объекта» реализуйте задачу при помощи адаптера класса.
Сравните оба решения. Отметьте их достоинства и недостатки.

#### **Бонус – 40 баллов за unit-тестирование адаптера**

Бонус начисляется за покрытие функционала разработанного адаптера объекта автоматическими тестами

#### ***Бонус – 10 баллов за UML-диаграмму классов***

В дополнение к диаграмме классов из задания «Адаптер объекта» нарисуйте диаграмму классов

### **Задание 3 – Поддержка цветных изображений – 60 баллов**

Разработчики библиотеки **graphics_lib** решили поддержать в ней возможность рисования цветных примитивов и доработали возможности холста.

```cpp
namespace graphics_lib
{
// Холст для рисования
class ICanvas
{
public:
  // Установка цвета в формате 0xRRGGBB
  virtual void SetColor(uint32_t rgbColor) = 0;
  virtual void MoveTo(int x, int y) = 0;
  virtual void LineTo(int x, int y) = 0;
  virtual ~ICanvas() = default;
};
// Реализация холста для рисования
class CCanvas : public ICanvas
{
public:
  void SetColor(uint32_t rgbColor) override
  {
    // TODO: вывести в output цвет в виде строки SetColor (#RRGGBB)
  }
  void MoveTo(int x, int y) override
  {
    // Реализация остается без изменения
  }
  void LineTo(int x, int y) override
  {
    // Реализация остается без изменения
  }
};
}
```

Разработчики библиотеки **shape_drawing_lib** также поддержали возможность создания цветных фигур –   конструктор прямоугольников и треугольников принимает опциональный параметр color (тип uint32_t), равный по умолчанию 0x000000.

Разработчики modern_graphics_lib также поддержали цвета в своей библиотеке, но, в отличие от graphics_lib, поддерживающей цвет в формате **#RRGGBB**, поддерживает цвет в формате RGBA, причем каждый компонент цвета принимает значение с плавающей запятой в диапазоне от 0.0 до 1.0. Четвертый компонент цвета задает альфа-составляющую, где значение 0.0 соответствует полностью прозрачному цвету а 1.0 – полностью непрозрачному:

```cpp
namespace modern_graphics_lib
{
class CPoint
{
  // Реализация остается без изменения
};
// Цвет в формате RGBA, каждый компонент принимает значения от 0.0f до 1.0f
class CRGBAColor
{
public:
  CRGBAColor(float r, float g, float b, float a):r(r), g(g), b(b), a(a){}
  float r, g, b, a;
}
// Класс для современного рисования графики
class CModernGraphicsRenderer
{
public:
  CModernGraphicsRenderer(ostream & strm);
  ~CModernGraphicsRenderer();
  void BeginDraw();
  // Выполняет рисование линии
  void DrawLine(const CPoint & start, const CPoint & end, const CRGBAColor& color)
  {
    // TODO: выводит в output инструкцию для рисования линии. Пример:
    // <line fromX="3" fromY="5" toX="5" toY="17">
    //   <color r="0.35" g="0.47" b="1.0" a="1.0" />
    // </line>
    // Можно вызывать только между BeginDraw() и EndDraw()
  }
  void EndDraw();
private:
  // Реализация остается без изменений
};
}
```

Допишите недостающий код для библиотек graphics_lib, shape_drawing_lib и modern_graphics_lib. Поддержите в приложении цветные фигуры, доработайте соответствующим образом адаптер холста.

#### ***Бонус – 30 баллов за unit-тестирование адаптера***

Бонус начисляется за unit-тестирование адаптера
