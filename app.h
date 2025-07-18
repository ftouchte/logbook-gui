/**************************************
 * logbook-gui, GUI app
 *
 * check the layout below
 * 
 * @author ftouchte
 * @date July 17, 2025
 * **********************************/
 
#ifndef LOGBOOK_GUI_APP
#define LOGBOOK_GUI_APP

#include <gtkmm.h>
#include <string>
#include <vector>
#include <cmath>

class Window : public Gtk::Window {
protected :
	// Widgets
	Gtk::Box VBox_main, VBox_header, VBox_body, HBox_footer;
	Gtk::Paned HPaned1, HPaned2;
	Gtk::Frame Frame_title, Frame_email, Frame_marker, Frame_screenshots, Frame_comments;
	Gtk::ScrolledWindow ScrolledWindow_comments;
	//Gtk::Label Label_settings;
	//Gtk::Separator Separator_sidebar;
	
	Gtk::Box VBox_screenshots, HBox_screenshots;
	Gtk::Notebook Notebook_screenshots;
	Gtk::Button Button_take_screenshot;
	Gtk::Button Button_plus;
	Gtk::Button Button_minus;
	Gtk::Button Button_submit;
	Gtk::Button Button_reset;

	Gtk::TextView TextView_comments;
	Glib::RefPtr<Gtk::TextBuffer> TextBuffer_comments;
	Gtk::Entry Entry_title, Entry_email, Entry_marker;

	
	// for CSS file	
	Glib::RefPtr<Gtk::CssProvider> m_refCssProvider; // copied from gtkmm documention
	static void on_parsing_error(const Glib::RefPtr<const Gtk::CssSection>& section, const Glib::Error& error);
public :
	Window();
	~Window();

	// Signals
	//void on_button_prev_clicked();
	//void on_mouse_clicked(int n_press, double x, double y);
};

#endif


