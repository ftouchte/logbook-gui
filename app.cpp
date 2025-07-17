/**************************************
 * logbook-gui, GUI app
 *
 * check the layout below
 * 
 * @author ftouchte
 * @date July 17, 2025
 * **********************************/

#include "app.h"

#include <string>
#include <cmath>
#include <vector>
#include <functional>
#include <cstdio>
#include <cstdlib>
#include <iostream>

/** Constructor */
Window::Window() :
	// Initialisation // Take of the order of declaration
	VBox_main(Gtk::Orientation::VERTICAL),
	VBox_header(Gtk::Orientation::VERTICAL,10),
	VBox_body(Gtk::Orientation::VERTICAL, 2),
	HBox_footer(Gtk::Orientation::HORIZONTAL,10),
	HPaned1(Gtk::Orientation::HORIZONTAL),
	HPaned2(Gtk::Orientation::HORIZONTAL)
	//Separator_sidebar(Gtk::Orientation::HORIZONTAL)
{
	set_title("logbook-gui");
	set_default_size(800,500);
	set_child(VBox_main);
	VBox_main.append(VBox_header);
	VBox_main.append(VBox_body);
	VBox_main.append(HBox_footer);

	// Header
	VBox_header.add_css_class("bar");
	// Body overview
	VBox_body.append(Frame_title);
	Frame_title.set_name("frame-title");
	Frame_title.set_label("Title");
	Frame_title.add_css_class("frame-layout");
	VBox_body.append(HPaned1);
	HPaned1.set_start_child(Frame_email);
	Frame_email.set_name("frame-email");
	Frame_email.set_label("Email");
	Frame_email.add_css_class("frame-layout");
	HPaned1.set_position(400); // requested size for the first child
	HPaned1.set_end_child(Frame_marker);
	Frame_marker.set_name("frame-marker");
	Frame_marker.set_label("Marker");
	Frame_marker.add_css_class("frame-layout");
	VBox_body.append(HPaned2);
	HPaned2.set_start_child(Frame_screenshots);
	Frame_screenshots.set_name("frame-screenshots");
	Frame_screenshots.set_label("Screenshots");
	Frame_screenshots.add_css_class("frame-layout");
	Frame_screenshots.set_expand();
	HPaned2.set_end_child(Frame_comments);
	Frame_comments.set_name("frame-comments");
	Frame_comments.set_label("Comments");
	Frame_comments.add_css_class("frame-layout");
	Frame_comments.set_expand();
	// Footer
	HBox_footer.append(Button_submit);
	Button_submit.add_css_class("button-layout");
	Button_submit.set_margin_start(2);
	Button_submit.set_margin_top(10);
	Button_submit.set_margin_bottom(5);
	Button_submit.set_child(*Gtk::make_managed<Gtk::Label>("Submit", Gtk::Align::CENTER));
	HBox_footer.append(Button_reset);
	Button_reset.set_child(*Gtk::make_managed<Gtk::Label>("Reset", Gtk::Align::CENTER));
	Button_reset.set_margin_top(10);
	Button_reset.set_margin_bottom(5);
	Button_reset.add_css_class("button-layout");
	

	//Button_select_file.set_margin_top(20);
	//Button_select_file.set_child(*Gtk::make_managed<Gtk::Label>("Select file", Gtk::Align::CENTER));
	//Button_select_file.add_css_class("button-layout");

	// Load extra CSS file (code copied from https://gnome.pages.gitlab.gnome.org/gtkmm-documentation/sec-custom-css-names.html)
	m_refCssProvider = Gtk::CssProvider::create();
#if HAS_STYLE_PROVIDER_ADD_PROVIDER_FOR_DISPLAY
	Gtk::StyleProvider::add_provider_for_display(get_display(), m_refCssProvider,
	GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
#else
	Gtk::StyleContext::add_provider_for_display(get_display(), m_refCssProvider,
	GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
#endif
	m_refCssProvider->signal_parsing_error().connect(
    		[](const auto& section, const auto& error)
    		{ on_parsing_error(section, error); }
  	);
	m_refCssProvider->load_from_path("custom_gtkmm.css");
	// end load extra CSS file



}


Window::~Window() {
	// do nothing
}

void Window::on_parsing_error(const Glib::RefPtr<const Gtk::CssSection>& section, const Glib::Error& error)
{ // gtkmm documention, load css file
	std::cerr << "on_parsing_error(): " << error.what() << std::endl;
	if (section)
	{
		const auto file = section->get_file();
		if (file)
		{
			std::cerr << "  URI = " << file->get_uri() << std::endl;
		}

	auto start_location = section->get_start_location();
	auto end_location = section->get_end_location();
	std::cerr << "  start_line = " << start_location.get_lines()+1
	      << ", end_line = " << end_location.get_lines()+1 << std::endl;
	std::cerr << "  start_position = " << start_location.get_line_chars()
	      << ", end_position = " << end_location.get_line_chars() << std::endl;
	}
}

/** Main function */
int main (int argc, char * argv[]) {
	printf("Start logbook-gui...\n");

	auto app = Gtk::Application::create("org.gtkmm.example");

	return app->make_window_and_run<Window>(argc, argv);
}



