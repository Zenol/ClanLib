/*
**  ClanLib SDK
**  Copyright (c) 1997-2012 The ClanLib Team
**
**  This software is provided 'as-is', without any express or implied
**  warranty.  In no event will the authors be held liable for any damages
**  arising from the use of this software.
**
**  Permission is granted to anyone to use this software for any purpose,
**  including commercial applications, and to alter it and redistribute it
**  freely, subject to the following restrictions:
**
**  1. The origin of this software must not be misrepresented; you must not
**     claim that you wrote the original software. If you use this software
**     in a product, an acknowledgment in the product documentation would be
**     appreciated but is not required.
**  2. Altered source versions must be plainly marked as such, and must not be
**     misrepresented as being the original software.
**  3. This notice may not be removed or altered from any source distribution.
**
**  Note: Some of the libraries ClanLib may link to may have additional
**  requirements or restrictions.
**
**  File Author(s):
**
**    Arkadiusz Kalinowski
*/

#include "precomp.h"
#include "basicphysics.h"

// The start of the Application
int BasicPhysics::start(const std::vector<std::string> &args)
{
	quit = false;

	int window_x_size = 640;
	int window_y_size = 480;
	// Set the window
	DisplayWindowDescription desc;
	desc.set_title("ClanLib BasicPhysics Example");
	desc.set_size(Size(window_x_size, window_y_size), true);
	desc.set_allow_resize(true);

	DisplayWindow window(desc);
	
	//Setup physic world
	PhysicWorldDescription phys_desc;
	phys_desc.set_gravity(0.0f,20.0f);
	phys_desc.set_sleep(true);
	phys_desc.set_timestep(1.0f/60.0f);
	phys_desc.set_velocity_iterations(8);
	phys_desc.set_position_iterations(3);

	PhysicWorld phys_world(phys_desc);

	//Setup ground body
	BodyDescription ground_desc;
	ground_desc.set_position(Vec2f(window_x_size/2.0f,window_y_size));
	ground_desc.set_type(body_static);

	Body ground(phys_world,ground_desc);

	//Setup ground fixture
	PolygonShape ground_shape;
	ground_shape.SetAsBox(window_x_size,20.0f,Vec2f(0.0f,0.0f),Angle(0,angle_degrees));

	FixtureDescription fixture_desc;
	fixture_desc.set_shape(ground_shape);
	
	ground.add_fixture(fixture_desc);

	//Setup box body
	BodyDescription box_desc;
	box_desc.set_position(Vec2f(50.0f,100.0f));
	box_desc.set_type(body_dynamic);
	box_desc.set_linear_velocity(Vec2f(20.0f,0.0f));

	//Setup box fixture
	PolygonShape box_shape;
	box_shape.SetAsBox(5.0f,5.0f,Vec2f(0.0f,0.0f),Angle(0,angle_degrees));

	FixtureDescription fixture_desc2;
	fixture_desc2.set_shape(ground_shape);
	fixture_desc2.set_restitution(0.5f);
	fixture_desc2.set_friction(0.0f);

	Body box(phys_world,box_desc);

	box.add_fixture(fixture_desc2);

	// Connect the Window close event
	Slot slot_quit = window.sig_window_close().connect(this, &BasicPhysics::on_window_close);

	// Connect a keyboard handler to on_key_up()
	Slot slot_input_up = (window.get_ic().get_keyboard()).sig_key_up().connect(this, &BasicPhysics::on_input_up);

	// Create the canvas
	Canvas canvas(window);

	// Load a sprite from a png-file
	Image spr_logo(canvas, "tux_small.png");
	spr_logo.set_alignment(origin_center);

	Vec2f ground_pos = ground.get_position();

	float sin_count = 0.0f;
	float ypos = 0.0f;
	float ydir = 0.3f;

	unsigned int last_time = System::get_time();

	// Run until someone presses escape
	while (!quit)
	{
		unsigned int current_time = System::get_time();
		float time_delta_ms = static_cast<float> (current_time - last_time);
		last_time = current_time;

		canvas.clear();
		canvas.fill(0.0f,window_y_size,window_x_size,window_y_size-20,Colorf::crimson);

		phys_world.step();
		Vec2f pos = box.get_position();
		
		spr_logo.draw(canvas,pos.x,pos.y);

		canvas.flush();
		window.flip(1);

		// This call processes user input and other events
		KeepAlive::process(0);

		System::sleep(6);
	}

	return 0;
}

// A key was pressed
void BasicPhysics::on_input_up(const InputEvent &key)
{
	if(key.id == keycode_escape)
	{
		quit = true;
	}
}

// The window was closed
void BasicPhysics::on_window_close()
{
	quit = true;
}


