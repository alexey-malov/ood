#include "editor_window.h"
#include <iostream>

static std::unique_ptr<IShape> RectAt(float x)
{
	return std::make_unique<Rectangle>(Rect{ x, 0, 10, 10 });
}

void Test()
{
	Drawing drawing;
	int notifications = 0;
	std::vector<size_t> countsSeen;
	auto subscription = drawing.DoOnChange([&] {
		++notifications;
		countsSeen.push_back(drawing.GetShapeCount());
	});

	// Without a batch: notify immediately.
	drawing.AddShape(RectAt(0));
	assert(notifications == 1 && countsSeen.back() == 1);
	drawing.GetShape(0).MoveBy({ 1, 0 });
	assert(notifications == 2);

	// Empty/no-op batches do not notify.
	drawing.WithDeferredNotifications([&] {});
	drawing.WithDeferredNotifications([&] { drawing.GetShape(0).MoveBy({ 0, 0 }); });
	assert(notifications == 2);

	// A batch coalesces additions and shape updates, and is nesting-safe.
	drawing.WithDeferredNotifications([&] {
		drawing.AddShape(RectAt(20));
		drawing.GetShape(0).MoveBy({ 2, 0 });
		assert(notifications == 2);
		drawing.WithDeferredNotifications([&] {
			drawing.AddShape(RectAt(40));
			drawing.GetShape(1).MoveBy({ 1, 1 });
		});
		assert(notifications == 2);
		drawing.GetShape(2).MoveBy({ 1, 0 });
	});
	assert(notifications == 3);
	assert(drawing.GetShapeCount() == 3 && countsSeen.back() == 3);

	// Direct shape subscribers still see individual changes.
	int directChanges = 0;
	auto shapeSubscription = drawing.GetShape(0).DoOnChange([&] { ++directChanges; });
	drawing.WithDeferredNotifications([&] {
		drawing.GetShape(0).MoveBy({ 3, 0 });
		drawing.GetShape(0).MoveBy({ 4, 0 });
	});
	assert(directChanges == 2 && notifications == 4);

	// If an action throws after modification, observers still learn about it.
	try
	{
		drawing.WithDeferredNotifications([&] {
			drawing.AddShape(RectAt(60));
			throw std::runtime_error("action failed");
		});
		assert(false);
	}
	catch (const std::runtime_error& e)
	{
		assert(std::string(e.what()) == "action failed");
	}
	assert(drawing.GetShapeCount() == 4 && notifications == 5);

	// Even when both the action and a subscriber throw, action error wins.
	auto throwingSubscriber = drawing.DoOnChange([] { throw std::runtime_error("observer failed"); });
	try
	{
		drawing.WithDeferredNotifications([&] {
			drawing.GetShape(0).MoveBy({ 1, 0 });
			throw std::runtime_error("action failed again");
		});
		assert(false);
	}
	catch (const std::runtime_error& e)
	{
		assert(std::string(e.what()) == "action failed again");
	}
	throwingSubscriber.disconnect();

	// The next change is notified immediately (no stuck batch depth).
	int before = notifications;
	drawing.GetShape(0).MoveBy({ 1, 0 });
	assert(notifications == before + 1);

	// A subscriber exception at normal batch completion propagates;
	// the batch state is already reset.
	auto throwingSubscriber2 = drawing.DoOnChange([] { throw std::logic_error("subscriber failed"); });
	try
	{
		drawing.WithDeferredNotifications([&] { drawing.GetShape(0).MoveBy({ 1, 0 }); });
		assert(false);
	}
	catch (const std::logic_error& e)
	{
		assert(std::string(e.what()) == "subscriber failed");
	}
	throwingSubscriber2.disconnect();
	before = notifications;
	drawing.GetShape(0).MoveBy({ 1, 0 });
	assert(notifications == before + 1);

	std::cout << "Drawing notification tests passed\n";
}

int main()
{
	Test();

	DrawingDocument document;

	Drawing& drawing = document.GetDrawing();
	drawing.AddShape(std::make_unique<Rectangle>(Rect{ 100.f, 100.f, 200.f, 100.f }));
	drawing.AddShape(std::make_unique<Ellipse>(Rect{ 400.f, 300.f, 150.f, 100.f }));

	EditorWindow window{ document };

	window.Run();
}
