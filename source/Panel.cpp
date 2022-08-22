/* Panel.cpp
Copyright (c) 2014 by Michael Zahniser

Endless Sky is free software: you can redistribute it and/or modify it under the
terms of the GNU General Public License as published by the Free Software
Foundation, either version 3 of the License, or (at your option) any later version.

Endless Sky is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more details.
*/

#include "Panel.h"

#include "Color.h"
#include "Command.h"
#include "Dialog.h"
#include "FillShader.h"
#include "GameData.h"
#include "Point.h"
#include "Preferences.h"
#include "Screen.h"
#include "UI.h"

#include <cmath>

using namespace std;

namespace {
	const set<Uint8> CONTROLLER_BUTTONS{
		SDL_CONTROLLER_BUTTON_A,
		SDL_CONTROLLER_BUTTON_B,
		SDL_CONTROLLER_BUTTON_Y,
		SDL_CONTROLLER_BUTTON_BACK,
		SDL_CONTROLLER_BUTTON_RIGHTSTICK,
		SDL_CONTROLLER_BUTTON_RIGHTSHOULDER,
		SDL_CONTROLLER_BUTTON_LEFTSHOULDER,
		SDL_CONTROLLER_BUTTON_DPAD_UP,
		SDL_CONTROLLER_BUTTON_DPAD_DOWN,
		SDL_CONTROLLER_BUTTON_DPAD_LEFT,
		SDL_CONTROLLER_BUTTON_DPAD_RIGHT
	};
}



// Move the state of this panel forward one game step.
void Panel::Step()
{
	// It is ok for panels to be stateless.
}



// Return true if this is a full-screen panel, so there is no point in
// drawing any of the panels under it.
bool Panel::IsFullScreen() const noexcept
{
	return isFullScreen;
}



// Return true if, when this panel is on the stack, no events should be
// passed to any panel under it. By default, all panels do this.
bool Panel::TrapAllEvents() const noexcept
{
	return trapAllEvents;
}



// Check if this panel can be "interrupted" to return to the main menu.
bool Panel::IsInterruptible() const noexcept
{
	return isInterruptible;
}



// Clear the list of clickable zones.
void Panel::ClearZones()
{
	zones.clear();
}



// Add a clickable zone to the panel.
void Panel::AddZone(const Rectangle &rect, const function<void()> &fun)
{
	// The most recently added zone will typically correspond to what was drawn
	// most recently, so it should be on top.
	zones.emplace_front(rect, fun);
}



void Panel::AddZone(const Rectangle &rect, SDL_Keycode key)
{
	AddZone(rect, [this, key](){ this->KeyDown(key, 0, Command(), true); });
}



// Check if a click at the given coordinates triggers a clickable zone. If
// so, apply that zone's action and return true.
bool Panel::ZoneClick(const Point &point)
{
	for(const Zone &zone : zones)
		if(zone.Contains(point))
		{
			// If the panel is in editing mode, make sure it knows that a mouse
			// click has broken it out of that mode, so it doesn't interpret a
			// button press and a text character entered.
			EndEditing();
			zone.Click();
			return true;
		}
	return false;
}



// Forward the given TestContext to the Engine under MainPanel.
void Panel::SetTestContext(TestContext &testContext)
{
}



// Panels will by default not allow fast-forward. The ones that do allow
// it will override this (virtual) function and return true.
bool Panel::AllowsFastForward() const noexcept
{
	return false;
}



// Only override the ones you need; the default action is to return false.
bool Panel::KeyDown(SDL_Keycode key, Uint16 mod, const Command &command, bool isNewPress)
{
	return false;
}



bool Panel::Click(int x, int y, int clicks)
{
	return false;
}



bool Panel::RClick(int x, int y)
{
	return false;
}



bool Panel::Hover(int x, int y)
{
	return false;
}



bool Panel::Drag(double dx, double dy)
{
	return false;
}



bool Panel::Scroll(double dx, double dy)
{
	return false;
}



bool Panel::Release(int x, int y)
{
	return false;
}



// Generic panel controller handler.
bool Panel::GamePadState(GamePad &controller)
{
	auto pressed = controller.HeldButtonsSince();
	auto released = controller.ReleasedButtons();
	set<Uint8> unhandledButtons;

	Point mouse = GetUI()->GetMouse();
	for(auto it = released.cbegin(); it != released.cend(); ++it)
	{
		if(it->first == SDL_CONTROLLER_BUTTON_A)
			Release(mouse.X(), mouse.Y());
	}
	for(auto it = pressed.cbegin(); it != pressed.cend(); ++it)
	{
		bool handled = false;
		if(it->first == SDL_CONTROLLER_BUTTON_A && it->second != controllerClickHandled)
		{
			controllerClickHandled = it->second;
			if(!ZoneClick(mouse))
				handled = Click(mouse.X(), mouse.Y(), 1);
			else
				handled = true;
		}
		else if(it->first == SDL_CONTROLLER_BUTTON_B)
			handled = Click(mouse.X(), mouse.Y(), 2);
		else if(it->first == SDL_CONTROLLER_BUTTON_Y)
			handled = RClick(mouse.X(), mouse.Y());
		else if(it->first == SDL_CONTROLLER_BUTTON_BACK)
		{
			DoKey(SDLK_ESCAPE);
			handled = true;
		}
		else if(it->first == SDL_CONTROLLER_BUTTON_RIGHTSTICK)
		{
			UI::MoveMouseOffset(Point(0, 0));
			handled = true;
		}
		else if(it->first == SDL_CONTROLLER_BUTTON_RIGHTSHOULDER)
		{
			GetUI()->CursorToNextZone(mouse);
			handled = true;
		}
		else if(it->first == SDL_CONTROLLER_BUTTON_LEFTSHOULDER)
		{
			GetUI()->CursorToPrevZone(mouse);
			handled = true;
		}
		if(!handled)
			unhandledButtons.insert(it->first);
	}
	Point leftStick = controller.LeftStick();
	if(leftStick.LengthSquared() > 0.05)
	{
		double x, y;
		Point move(
			pow(leftStick.X() * GamePad::STICK_MOUSE_MULT, 3), pow(leftStick.Y() * GamePad::STICK_MOUSE_MULT, 3));
		move += controllerCursorRem;
		controllerCursorRem.Set(modf(move.X(), &x), modf(move.Y(), &y));
		if(pressed.find(SDL_CONTROLLER_BUTTON_LEFTSTICK) != pressed.cend())
			Drag(x, y);
		else
		{
			GetUI()->MoveMouseRelative(Point(x, y));
			if(pressed.find(SDL_CONTROLLER_BUTTON_A) != pressed.cend())
				Drag(x, y);
		}
	}
	double rightStickY = controller.RightStickY();
	if(rightStickY > 0.5)
		Scroll(0, -rightStickY + GamePad::SCROLL_THRESHOLD);
	else if(rightStickY < -0.5)
		Scroll(0, -rightStickY - GamePad::SCROLL_THRESHOLD);

	// Leave pressed state for the parent panel handler
	for(auto it = pressed.begin(); it != pressed.end();)
	{
		if(unhandledButtons.find(it->first) != unhandledButtons.cend())
			it = pressed.erase(it);
		else
			++it;
	}
	// Special case, button A needs to stay in the state object for dragging.
	pressed.erase(SDL_CONTROLLER_BUTTON_A);
	if(!pressed.empty())
	{
		set<Uint8> pressedKeyIds;
		for(auto it = pressed.cbegin(); it != pressed.cend(); ++it)
			pressedKeyIds.insert(it->first);
		controller.Clear(pressedKeyIds);
	}

	if(controller.RepeatButton(SDL_CONTROLLER_BUTTON_DPAD_UP))
		DoKey(SDLK_UP);
	if(controller.RepeatButton(SDL_CONTROLLER_BUTTON_DPAD_DOWN))
		DoKey(SDLK_DOWN);
	if(controller.RepeatButton(SDL_CONTROLLER_BUTTON_DPAD_LEFT))
		DoKey(SDLK_LEFT);
	if(controller.RepeatButton(SDL_CONTROLLER_BUTTON_DPAD_RIGHT))
		DoKey(SDLK_RIGHT);
	if(controller.RepeatAxis(SDL_CONTROLLER_AXIS_RIGHTX))
		GetUI()->NextPanel(true);
	else if(controller.RepeatAxisNeg(SDL_CONTROLLER_AXIS_RIGHTX))
		GetUI()->NextPanel(false);

	return unhandledButtons.empty();
}



void Panel::SetIsFullScreen(bool set)
{
	isFullScreen = set;
}



void Panel::SetTrapAllEvents(bool set)
{
	trapAllEvents = set;
}



void Panel::SetInterruptible(bool set)
{
	isInterruptible = set;
}



// Dim the background of this panel.
void Panel::DrawBackdrop() const
{
	if(!GetUI()->IsTop(this))
		return;

	// Darken everything but the dialog.
	const Color &back = *GameData::Colors().Get("dialog backdrop");
	FillShader::Fill(Point(), Point(Screen::Width(), Screen::Height()), back);
}



bool Panel::NextPanel()
{
	return false;
}



bool Panel::PrevPanel()
{
	return false;
}



UI *Panel::GetUI() const noexcept
{
	return ui;
}



// This is not for overriding, but for calling KeyDown with only one or two
// arguments. In this form, the command is never set, so you can call this
// with a key representing a known keyboard shortcut without worrying that a
// user-defined command key will override it.
bool Panel::DoKey(SDL_Keycode key, Uint16 mod)
{
	return KeyDown(key, mod, Command(), true);
}



// A lot of different UI elements allow a modifier to change the number of
// something you are buying, so the shared function is defined here:
int Panel::Modifier()
{
	SDL_Keymod mod = SDL_GetModState();

	int modifier = 1;
	if(mod & KMOD_ALT)
		modifier *= 500;
	if(mod & (KMOD_CTRL | KMOD_GUI))
		modifier *= 20;
	if(mod & KMOD_SHIFT)
		modifier *= 5;

	double rightTrigger = GamePad::Singleton().RightTrigger();
	if(rightTrigger > 0.8)
		modifier *= 500;
	else if(rightTrigger > 0.4)
		modifier *= 20;
	else if(rightTrigger > 0.05)
		modifier *= 5;

	return modifier;
}



// Display the given help message if it has not yet been shown. Return true
// if the message was displayed.
bool Panel::DoHelp(const string &name) const
{
	string preference = "help: " + name;
	if(Preferences::Has(preference))
		return false;

	const string &message = GameData::HelpMessage(name);
	if(message.empty())
		return false;

	Preferences::Set(preference);
	ui->Push(new Dialog(message));

	return true;
}



void Panel::SetUI(UI *ui)
{
	this->ui = ui;
}



void Panel::CursorToFirstZone()
{
	auto firstZone = zones.cbegin();
	if(firstZone != zones.cend())
	{
		Point center = firstZone->Center();
		GetUI()->MoveMouseOffset(center);
	}
}
