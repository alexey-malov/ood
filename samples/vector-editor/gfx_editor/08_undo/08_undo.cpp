#include "editor_window.h"

int main()
{
	DrawingDocument document;

	Drawing& drawing = document.GetDrawing();
	drawing.AddShape(std::make_unique<Rectangle>(Rect{ 100.f, 100.f, 200.f, 100.f }));
	drawing.AddShape(std::make_unique<Ellipse>(Rect{ 400.f, 300.f, 150.f, 100.f }));

	EditorWindow window{ document };

	window.Run();
}
