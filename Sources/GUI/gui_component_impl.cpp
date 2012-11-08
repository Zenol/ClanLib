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
**    Magnus Norddahl
**    Harry Storbacka
*/

#include "GUI/precomp.h"
#include "API/GUI/gui_manager.h"
#include "API/GUI/gui_component.h"
#include "API/GUI/gui_message_pointer.h"
#include "API/Display/2D/image.h"
#include "gui_component_impl.h"
#include "gui_manager_impl.h"
#include "gui_css_strings.h"
#include "css_clan_box_math.h"

namespace clan
{

GUIComponent_Impl::GUIComponent_Impl(const std::shared_ptr<GUIManager_Impl> &init_gui_manager, GUIComponent *parent_or_owner, bool toplevel)
: gui_manager(init_gui_manager), parent(0), prev_sibling(0), next_sibling(0), first_child(0), last_child(0),
  focus_policy(GUIComponent::focus_refuse), allow_resize(false), clip_children(false), enabled(true),
  visible(true), activated(false), default_handler(false), cancel_handler(false),
  constant_repaint(false), blocks_default_action_when_focused(false), is_selected_in_group(false), double_click_enabled(true), pointer_inside_component(false)
{
	gui_manager_impl = gui_manager.lock().get();

	if (!toplevel)
	{
		parent = parent_or_owner;
		//css_element = parent->get_top_level_component()->impl->css_layout.create_element("component");
		//parent->impl->css_element.append_child(css_element);
	}
	else
	{
		css_element = css_layout.create_element("component");
		css_element.apply_properties("display: block");
		css_layout.set_root_element(css_element);
		css_layout.func_get_image().set(this, &GUIComponent_Impl::on_css_layout_get_image);
	}

	func_process_message.set(this, &GUIComponent_Impl::on_process_message);
}

GUIComponent_Impl *GUIComponent_Impl::create_from_parent(GUIComponent *parent)
{
	if (parent)
		return new GUIComponent_Impl(parent->get_gui_manager().impl, parent, false);
	else
		throw Exception("Cannot create child component with no parent!");
}

GUIComponent_Impl *GUIComponent_Impl::create_from_manager(GUIManager *manager)
{
	if (manager)
		return new GUIComponent_Impl(manager->impl, 0, true);
	else
		throw Exception("Cannot create component with a null manager!");
}

GUIComponent_Impl *GUIComponent_Impl::create_from_owner(GUIComponent *owner)
{
	if (owner)
		return new GUIComponent_Impl(owner->get_gui_manager().impl, owner, true);
	else
		throw Exception("Cannot create child component with a null owner!");
}

GUIComponent_Impl::~GUIComponent_Impl()
{
	while (last_child)
		delete last_child;

	if (prev_sibling)
		prev_sibling->impl->next_sibling = next_sibling;
	if (next_sibling)
		next_sibling->impl->prev_sibling = prev_sibling;
	
	if (prev_sibling == 0 && parent)
		parent->impl->first_child = next_sibling;
	if (next_sibling == 0 && parent)
		parent->impl->last_child = prev_sibling;

	gui_manager_impl->remove_component(this);
}

void GUIComponent_Impl::set_geometry(Rect new_geometry, bool client_area)
{
	if (parent == 0)
	{
		GUITopLevelWindow *handle = gui_manager.lock()->get_toplevel_window(component);
		gui_manager.lock()->window_manager.set_geometry(handle, new_geometry, client_area);
		new_geometry = gui_manager.lock()->window_manager.get_geometry(handle, true);
	}

	// repaint parent at old geometry
	if (component->get_parent_component())
		component->get_parent_component()->request_repaint(geometry);

	// Check for resize
	if ((geometry.get_width() != new_geometry.get_width()) || (geometry.get_height() != new_geometry.get_height()) )
	{
		geometry = new_geometry;
		geometry_updated();
	}
	else
	{
		geometry = new_geometry;
		component->request_repaint();
	}
}

void GUIComponent_Impl::geometry_updated()
{
	if (!layout.is_null())
		layout.set_geometry(geometry.get_size());

	if (!css_layout.is_null())
	{
		Canvas canvas = component->get_canvas();
		css_layout.layout(canvas, geometry.get_size());
	}

	if (!func_resized.is_null())
		func_resized.invoke();

	component->request_repaint();
}

void GUIComponent_Impl::invoke_enablemode_changed()
{
	if (!func_enablemode_changed.is_null())
		func_enablemode_changed.invoke();

	GUIComponent *cur = first_child;
	while (cur)
	{
		cur->impl->invoke_enablemode_changed();
		cur = cur->get_next_sibling();
	}
}

Image GUIComponent_Impl::on_css_layout_get_image(Canvas &canvas, const std::string &url)
{
	try
	{
        ResourceManager resources = gui_manager.lock()->resources;
		return Image(canvas, url, &resources);
	}
	catch (Exception e)
	{
		// Hmm what to do about that?
		return Image();
	}
}

void GUIComponent_Impl::layout_content()
{
	// Root component needs to calculate its own used values
	if (!parent)
	{
		CSSClanBoxUsedValues initial_containing_box;
		initial_containing_box.width = geometry.get_width();
		initial_containing_box.height = geometry.get_height();

		CSSClanBoxInitialUsedValues::visit(css_used_values, css_properties, initial_containing_box);
		CSSClanBoxApplyMinMaxConstraints::visit(css_used_values, css_properties, initial_containing_box);

		if (css_used_values.width_undetermined)
		{
			css_used_values.width = initial_containing_box.width - css_used_values.margin.left - css_used_values.margin.right - css_used_values.border.left - css_used_values.border.right - css_used_values.padding.right - css_used_values.padding.right;
			css_used_values.width_undetermined = false;
		}

		// TBD: this isn't the default in normal CSS
		if (css_used_values.height_undetermined)
		{
			css_used_values.height = initial_containing_box.height - css_used_values.margin.top - css_used_values.margin.top - css_used_values.border.top - css_used_values.border.bottom - css_used_values.padding.bottom - css_used_values.padding.bottom;
			css_used_values.height_undetermined = false;
		}
	}

	switch (css_properties.display.type)
	{
	case CSSBoxDisplay::type_clan_box:
		layout_clan_box();
		break;
	case CSSBoxDisplay::type_clan_grid:
		layout_clan_grid();
		break;
	case CSSBoxDisplay::type_clan_stacked:
		layout_clan_stacked();
		break;
	default:
		throw Exception("Unsupported display type for GUI components");
	}

	for (GUIComponent *child = first_child; child != 0; child = child->get_next_sibling())
		child->impl->layout_content();

	for (GUIComponent *child = first_child; child != 0; child = child->get_next_sibling())
		child->impl->layout_absolute_or_fixed();
}

void GUIComponent_Impl::layout_absolute_or_fixed()
{
	if (css_properties.position.type == CSSBoxPosition::type_absolute || css_properties.position.type == CSSBoxPosition::type_fixed)
	{
		float containing_width = parent->get_width();
		float containing_height = parent->get_height();

		float left = 0.0f;
		if (css_properties.left.type == CSSBoxLeft::type_length)
			left = css_properties.left.length.value;
		else if (css_properties.left.type == CSSBoxLeft::type_percentage)
			left = css_properties.left.percentage * containing_width / 100.0f;

		float right = 0.0f;
		if (css_properties.right.type == CSSBoxRight::type_length)
			right = css_properties.right.length.value;
		else if (css_properties.right.type == CSSBoxRight::type_percentage)
			right = css_properties.right.percentage * containing_width / 100.0f;

		// to do: implement all the complicated rules from CSSLayoutTreeNode::layout_absolute_or_fixed

		layout_content();
	}
}

void GUIComponent_Impl::layout_clan_box()
{
	// -clan-box layout places child boxes horizontally or vertically one after another
	// -clan-box-direction controls the layout direction

	if (css_properties.clan_box_direction.type == CSSBoxClanBoxDirection::type_vertical)
	{
		layout_clan_box_vertical();
	}
	else if (css_properties.clan_box_direction.type == CSSBoxClanBoxDirection::type_horizontal)
	{
		layout_clan_box_horizontal();
	}
	else
	{
		throw Exception("Unexpected CSS -clan-box-direction computed value");
	}
}

void GUIComponent_Impl::layout_clan_box_horizontal()
{
	// Calculate min/preferred/max widths of all child boxes
	CSSClanBoxMath box_math;
	for (GUIComponent *child = first_child; child != 0; child = child->get_next_sibling())
	{
		if (child->get_css_properties().position.type != CSSBoxPosition::type_absolute && child->get_css_properties().position.type != CSSBoxPosition::type_fixed)
		{
			CSSClanBoxUsedValues &child_used_values = child->impl->css_used_values;

			// Start with top-down calculated values
			CSSClanBoxInitialUsedValues::visit(child_used_values, child->impl->css_properties, css_used_values);

			// If the width of the box cannot be determined from CSS, then ask the component:
			if (child_used_values.width_undetermined)
			{
				child_used_values.width = child->get_preferred_content_width();
				child_used_values.width_undetermined = false;
			}

			// Boxes should not shrink to less than the minimum shrink-to-fit size.
			// TBD: this rule only applies to shrink-to-fit boxes (i.e. floats with auto width) in normal CSS
			child_used_values.min_width = std::max(child_used_values.min_width, child->get_min_preferred_content_width());

			// Make sure width is within the min/max values:
			CSSClanBoxApplyMinMaxConstraints::visit(child_used_values, child->impl->css_properties, css_used_values);

			CSSUsedValue used_noncontent_width = 
				child_used_values.margin.left +
				child_used_values.border.left +
				child_used_values.padding.left +
				child_used_values.padding.right +
				child_used_values.border.right +
				child_used_values.margin.right;

			box_math.used_min_lengths.push_back(used_noncontent_width + child_used_values.min_width);
			box_math.used_lengths.push_back(used_noncontent_width + child_used_values.width);
			box_math.used_max_lengths.push_back(used_noncontent_width + child_used_values.max_width);

			switch (child->impl->css_properties.clan_box_width_shrink_factor.type)
			{
			default:
			case CSSBoxClanBoxSizingFactor::type_auto:
				box_math.used_shrink_weights.push_back(0.0f);
				break;
			case CSSBoxClanBoxSizingFactor::type_number:
				box_math.used_shrink_weights.push_back(child->impl->css_properties.clan_box_width_shrink_factor.number);
				break;
			}

			switch (child->impl->css_properties.clan_box_width_expand_factor.type)
			{
			default:
			case CSSBoxClanBoxSizingFactor::type_auto:
				box_math.used_expand_weights.push_back(0.0f);
				break;
			case CSSBoxClanBoxSizingFactor::type_number:
				box_math.used_expand_weights.push_back(child->impl->css_properties.clan_box_width_expand_factor.number);
				break;
			}
		}
	}

	// Adjust the widths of the boxes
	box_math.adjust(css_used_values.width);

	// Save adjusted width values and calculate the resulting box heights
	int i = 0;
	for (GUIComponent *child = first_child; child != 0; child = child->get_next_sibling(), i++)
	{
		if (child->get_css_properties().position.type != CSSBoxPosition::type_absolute && child->get_css_properties().position.type != CSSBoxPosition::type_fixed)
		{
			CSSClanBoxUsedValues &child_used_values = child->impl->css_used_values;

			// Save the result of the horizontal adjustment
			child_used_values.width = box_math.used_lengths[i];

			// If the height of the box could not be determined from CSS, then ask the component:
			if (child_used_values.height_undetermined)
			{
				child_used_values.height = child->get_preferred_content_height(child_used_values.width);
				child_used_values.height_undetermined = false;
			}

			// Make sure height is within the min/max values:
			CSSClanBoxApplyMinMaxConstraints::visit(child_used_values, child->impl->css_properties, css_used_values);

			CSSUsedValue used_noncontent_height = 
				child_used_values.margin.top +
				child_used_values.border.top +
				child_used_values.padding.top +
				child_used_values.padding.bottom +
				child_used_values.border.bottom +
				child_used_values.margin.bottom;

			// Adjust height of box based on the shrink/expand factors
			CSSClanBoxMath perpendicular_math;

			perpendicular_math.used_min_lengths.push_back(used_noncontent_height + child_used_values.min_height);
			perpendicular_math.used_lengths.push_back(used_noncontent_height + child_used_values.height);
			perpendicular_math.used_max_lengths.push_back(used_noncontent_height + child_used_values.max_height);

			switch (child->impl->css_properties.clan_box_height_shrink_factor.type)
			{
			default:
			case CSSBoxClanBoxSizingFactor::type_auto:
				perpendicular_math.used_shrink_weights.push_back(0.0f);
				break;
			case CSSBoxClanBoxSizingFactor::type_number:
				perpendicular_math.used_shrink_weights.push_back(child->impl->css_properties.clan_box_height_shrink_factor.number);
				break;
			}

			switch (child->impl->css_properties.clan_box_height_expand_factor.type)
			{
			default:
			case CSSBoxClanBoxSizingFactor::type_auto:
				perpendicular_math.used_expand_weights.push_back(0.0f);
				break;
			case CSSBoxClanBoxSizingFactor::type_number:
				perpendicular_math.used_expand_weights.push_back(child->impl->css_properties.clan_box_height_expand_factor.number);
				break;
			}

			perpendicular_math.adjust(css_used_values.height);

			// Save the result of the vertical adjustment
			child_used_values.height = box_math.used_lengths[i];
		}
	}

	// Set the actual geometry
	CSSUsedValue x = 0.0f;
	CSSUsedValue y = 0.0f;
	i = 0;
	for (GUIComponent *child = first_child; child != 0; child = child->get_next_sibling(), i++)
	{
		if (child->get_css_properties().position.type != CSSBoxPosition::type_absolute && child->get_css_properties().position.type != CSSBoxPosition::type_fixed)
		{
			CSSClanBoxUsedValues &child_used_values = child->impl->css_used_values;

			CSSUsedValue used_offset_x = child_used_values.margin.left + child_used_values.border.left + child_used_values.padding.left + child->impl->get_css_relative_x(css_used_values.width);
			CSSUsedValue used_offset_y = child_used_values.margin.top + child_used_values.border.top + child_used_values.padding.top + child->impl->get_css_relative_y(css_used_values.height);

			// Used to actual mapping
			CSSActualValue x1 = (CSSActualValue)(x + used_offset_x + child_used_values.margin.left);
			CSSActualValue y1 = (CSSActualValue)(y + used_offset_y + child_used_values.margin.top);
			CSSActualValue x2 = (CSSActualValue)(x + used_offset_x + child_used_values.width - child_used_values.margin.left - child_used_values.margin.right + 0.5f);
			CSSActualValue y2 = (CSSActualValue)(y + used_offset_y + child_used_values.height - child_used_values.margin.top - child_used_values.margin.bottom + 0.5f);
			child->set_geometry(Rect(x1, y1, x2, y2));

			x += child_used_values.margin.left + child_used_values.border.left + child_used_values.padding.left + child_used_values.width + child_used_values.padding.right + child_used_values.border.right + child_used_values.margin.right;
		}
	}
}

void GUIComponent_Impl::layout_clan_box_vertical()
{
	CSSClanBoxMath box_math;
	for (GUIComponent *child = first_child; child != 0; child = child->get_next_sibling())
	{
		if (child->get_css_properties().position.type != CSSBoxPosition::type_absolute && child->get_css_properties().position.type != CSSBoxPosition::type_fixed)
		{
			CSSClanBoxUsedValues &child_used_values = child->impl->css_used_values;

			// Start with top-down calculated values
			CSSClanBoxInitialUsedValues::visit(child_used_values, child->impl->css_properties, css_used_values);

			// If the width of the box cannot be determined from CSS, then ask the component:
			if (child_used_values.width_undetermined)
			{
				child_used_values.width = child->get_preferred_content_width();
				child_used_values.width_undetermined = false;
			}

			// Boxes should not shrink to less than the minimum shrink-to-fit size.
			// TBD: this rule only applies to shrink-to-fit boxes (i.e. floats with auto width) in normal CSS
			child_used_values.min_width = std::max(child_used_values.min_width, child->get_min_preferred_content_width());

			// Make sure width is within the min/max values:
			CSSClanBoxApplyMinMaxConstraints::visit(child_used_values, child->impl->css_properties, css_used_values);

			CSSUsedValue used_noncontent_width = 
				child_used_values.margin.left +
				child_used_values.border.left +
				child_used_values.padding.left +
				child_used_values.padding.right +
				child_used_values.border.right +
				child_used_values.margin.right;

			// Adjust width of box based on the shrink/expand factors
			CSSClanBoxMath perpendicular_math;

			perpendicular_math.used_min_lengths.push_back(used_noncontent_width + child_used_values.min_width);
			perpendicular_math.used_lengths.push_back(used_noncontent_width + child_used_values.width);
			perpendicular_math.used_max_lengths.push_back(used_noncontent_width + child_used_values.max_height);

			switch (child->impl->css_properties.clan_box_width_shrink_factor.type)
			{
			default:
			case CSSBoxClanBoxSizingFactor::type_auto:
				perpendicular_math.used_shrink_weights.push_back(0.0f);
				break;
			case CSSBoxClanBoxSizingFactor::type_number:
				perpendicular_math.used_shrink_weights.push_back(child->impl->css_properties.clan_box_width_shrink_factor.number);
				break;
			}

			switch (child->impl->css_properties.clan_box_width_expand_factor.type)
			{
			default:
			case CSSBoxClanBoxSizingFactor::type_auto:
				perpendicular_math.used_expand_weights.push_back(0.0f);
				break;
			case CSSBoxClanBoxSizingFactor::type_number:
				perpendicular_math.used_expand_weights.push_back(child->impl->css_properties.clan_box_width_expand_factor.number);
				break;
			}

			perpendicular_math.adjust(css_used_values.width);

			// Save the result of the horizontal adjustment
			child_used_values.width = perpendicular_math.used_lengths[0];

			// If the height of the box could not be determined from CSS, then ask the component:
			if (child_used_values.height_undetermined)
			{
				child_used_values.height = child->get_preferred_content_height(child_used_values.width);
				child_used_values.height_undetermined = false;
			}

			// Make sure height is within the min/max values:
			CSSClanBoxApplyMinMaxConstraints::visit(child_used_values, child->impl->css_properties, css_used_values);

			// Set up vertical box adjustment math:

			CSSUsedValue used_noncontent_height = 
				child_used_values.margin.top +
				child_used_values.border.top +
				child_used_values.padding.top +
				child_used_values.padding.bottom +
				child_used_values.border.bottom +
				child_used_values.margin.bottom;

			box_math.used_min_lengths.push_back(used_noncontent_height + child_used_values.min_height);
			box_math.used_lengths.push_back(used_noncontent_height + child_used_values.height);
			box_math.used_max_lengths.push_back(used_noncontent_height + child_used_values.max_height);

			switch (child->impl->css_properties.clan_box_height_shrink_factor.type)
			{
			default:
			case CSSBoxClanBoxSizingFactor::type_auto:
				box_math.used_shrink_weights.push_back(0.0f);
				break;
			case CSSBoxClanBoxSizingFactor::type_number:
				box_math.used_shrink_weights.push_back(child->impl->css_properties.clan_box_height_shrink_factor.number);
				break;
			}

			switch (child->impl->css_properties.clan_box_height_expand_factor.type)
			{
			default:
			case CSSBoxClanBoxSizingFactor::type_auto:
				box_math.used_expand_weights.push_back(0.0f);
				break;
			case CSSBoxClanBoxSizingFactor::type_number:
				box_math.used_expand_weights.push_back(child->impl->css_properties.clan_box_height_expand_factor.number);
				break;
			}
		}
	}

	// Adjust the heights of the boxes
	box_math.adjust(css_used_values.height);

	// Save adjusted height values
	int i = 0;
	for (GUIComponent *child = first_child; child != 0; child = child->get_next_sibling(), i++)
	{
		if (child->get_css_properties().position.type != CSSBoxPosition::type_absolute && child->get_css_properties().position.type != CSSBoxPosition::type_fixed)
		{
			CSSClanBoxUsedValues &child_used_values = child->impl->css_used_values;
			child_used_values.height = box_math.used_lengths[i];
		}
	}

	// Set the actual geometry
	CSSUsedValue x = 0.0f;
	CSSUsedValue y = 0.0f;
	i = 0;
	for (GUIComponent *child = first_child; child != 0; child = child->get_next_sibling(), i++)
	{
		if (child->get_css_properties().position.type != CSSBoxPosition::type_absolute && child->get_css_properties().position.type != CSSBoxPosition::type_fixed)
		{
			CSSClanBoxUsedValues &child_used_values = child->impl->css_used_values;

			CSSUsedValue used_offset_x = child_used_values.margin.left + child_used_values.border.left + child_used_values.padding.left + child->impl->get_css_relative_x(css_used_values.width);
			CSSUsedValue used_offset_y = child_used_values.margin.top + child_used_values.border.top + child_used_values.padding.top + child->impl->get_css_relative_y(css_used_values.height);

			// Used to actual mapping
			CSSActualValue x1 = (CSSActualValue)(x + used_offset_x + child_used_values.margin.left);
			CSSActualValue y1 = (CSSActualValue)(y + used_offset_y + child_used_values.margin.top);
			CSSActualValue x2 = (CSSActualValue)(x + used_offset_x + child_used_values.width - child_used_values.margin.left - child_used_values.margin.right + 0.5f);
			CSSActualValue y2 = (CSSActualValue)(y + used_offset_y + child_used_values.height - child_used_values.margin.top - child_used_values.margin.bottom + 0.5f);
			child->set_geometry(Rect(x1, y1, x2, y2));

			y += child_used_values.margin.top + child_used_values.border.top + child_used_values.padding.top + child_used_values.height + child_used_values.padding.bottom + child_used_values.border.bottom + child_used_values.margin.bottom;
		}
	}
}

void GUIComponent_Impl::layout_clan_grid()
{
	throw Exception("-clan-grid layout not implemented yet");
}

void GUIComponent_Impl::layout_clan_stacked()
{
	throw Exception("-clan-stacked layout not implemented yet");
}

float GUIComponent_Impl::get_css_relative_x(float containing_width)
{
	if (css_properties.position.type == CSSBoxPosition::type_relative)
	{
		if (css_properties.left.type == CSSBoxLeft::type_length)
			return css_properties.left.length.value;
		else if (css_properties.left.type == CSSBoxLeft::type_percentage)
			return css_properties.left.percentage / 100.0f * containing_width;
		else
			return 0.0f;
	}
	else
	{
		return 0.0f;
	}
}

float GUIComponent_Impl::get_css_relative_y(float containing_height)
{
	if (css_properties.position.type == CSSBoxPosition::type_relative)
	{
		if (css_properties.top.type == CSSBoxTop::type_length)
			return css_properties.top.length.value;
		else if (css_properties.top.type == CSSBoxTop::type_percentage)
			return css_properties.top.percentage / 100.0f * containing_height;
		else
			return 0.0f;
	}
	else
	{
		return 0.0f;
	}
}

void GUIComponent_Impl::on_process_message(std::shared_ptr<GUIMessage> &msg)
{
	std::shared_ptr<GUIMessage_Pointer> pointer = std::dynamic_pointer_cast<GUIMessage_Pointer>(msg);
	if (pointer)
	{
		if (pointer->pointer_type == GUIMessage_Pointer::pointer_enter)
		{
			component->set_pseudo_class(CssStr::hot, true);
			msg->consumed = true;
		}
		else if (pointer->pointer_type == GUIMessage_Pointer::pointer_leave)
		{
			component->set_pseudo_class(CssStr::hot, false);
			msg->consumed = true;
		}
	}
}

}
