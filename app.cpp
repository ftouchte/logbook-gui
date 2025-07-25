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
#include <ctime>
#include <regex>
#include <fstream>
#include <filesystem>
#include <iomanip>
#include <sqlite3.h>

/** Constructor */
Window::Window() :
	// Initialisation // Take of the order of declaration
	VBox_main(Gtk::Orientation::VERTICAL),
	VBox_header(Gtk::Orientation::VERTICAL,10),
	VBox_body(Gtk::Orientation::VERTICAL, 2),
	HBox_footer(Gtk::Orientation::HORIZONTAL,10),
	HPaned1(Gtk::Orientation::HORIZONTAL),
	HPaned2(Gtk::Orientation::HORIZONTAL),
	VBox_screenshots(Gtk::Orientation::VERTICAL,10),
	HBox_screenshots(Gtk::Orientation::HORIZONTAL,10)
	//Separator_sidebar(Gtk::Orientation::HORIZONTAL)
{
	set_title("logbook-gui");
	set_default_size(900,600);
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
	HPaned1.set_position(500); // requested size for the first child
	HPaned1.set_end_child(Frame_marker);
	Frame_marker.set_name("frame-marker");
	Frame_marker.set_label("Marker");
	Frame_marker.add_css_class("frame-layout");
	VBox_body.append(HPaned2);
		// Frame screenshots
	HPaned2.set_start_child(Frame_screenshots);
	HPaned2.set_position(450); 
	Frame_screenshots.set_name("frame-screenshots");
	Frame_screenshots.set_label("Screenshots");
	Frame_screenshots.add_css_class("frame-layout");
	Frame_screenshots.set_expand();
	Frame_screenshots.set_child(VBox_screenshots);
	VBox_screenshots.append(Notebook_screenshots);
	Notebook_screenshots.set_expand();
	Notebook_screenshots.set_name("notebook-screenshots");
	attachments.push_back("");
	Notebook_screenshots.append_page(*Gtk::make_managed<Gtk::Picture>(), "empty");
	VBox_screenshots.append(HBox_screenshots);
	HBox_screenshots.append(Button_take_screenshot);
	Button_take_screenshot.set_child(*Gtk::make_managed<Gtk::Label>("Take screenshot", Gtk::Align::CENTER));
	Button_take_screenshot.add_css_class("button-layout");
	HBox_screenshots.append(Button_plus);
	Button_plus.set_child(*Gtk::make_managed<Gtk::Label>("+", Gtk::Align::CENTER));
	Button_plus.add_css_class("button-layout");
	HBox_screenshots.append(Button_minus);
	Button_minus.set_child(*Gtk::make_managed<Gtk::Label>("-", Gtk::Align::CENTER));
	Button_minus.add_css_class("button-layout");
	HPaned2.set_end_child(Frame_comments);
	Frame_comments.set_name("frame-comments");
	Frame_comments.set_label("Comments");
	Frame_comments.add_css_class("frame-layout");
	Frame_comments.set_expand();
	// Footer
	HBox_footer.append(Button_submit);
	HBox_footer.add_css_class("background");
	Button_submit.add_css_class("button-layout");
	Button_submit.set_margin_start(18);
	Button_submit.set_margin_top(10);
	Button_submit.set_margin_bottom(5);
	Button_submit.set_child(*Gtk::make_managed<Gtk::Label>("Submit", Gtk::Align::CENTER));
	HBox_footer.append(Button_reset);
	Button_reset.set_child(*Gtk::make_managed<Gtk::Label>("Reset", Gtk::Align::CENTER));
	Button_reset.set_margin_top(10);
	Button_reset.set_margin_bottom(5);
	Button_reset.add_css_class("button-layout");
	HBox_footer.append(Label_log);
	Label_log.set_margin_start(20);

	// TextBuffer and Entry
	TextBuffer_comments = Gtk::TextBuffer::create();
	Frame_comments.set_child(TextView_comments);
	TextView_comments.set_buffer(TextBuffer_comments);
	TextView_comments.set_wrap_mode(Gtk::WrapMode::WORD);
	Frame_title.set_child(Entry_title);
	Frame_email.set_child(Entry_email);
	Frame_marker.set_child(Entry_marker);
	Entry_title.add_css_class("entry");
	Entry_email.add_css_class("entry");
	Entry_marker.add_css_class("entry");
	TextView_comments.add_css_class("entry");
	char buffer[100];
	int new_entry_number = get_last_lognumber() + 1;
	sprintf(buffer, "Entry n° %d: Enter a title for your log entry...", new_entry_number);
	Entry_title.set_text(buffer);

	// Define signals
	Button_take_screenshot.signal_clicked().connect( sigc::mem_fun(*this, &Window::on_button_take_screenshot));
	Button_plus.signal_clicked().connect( sigc::mem_fun(*this, &Window::on_button_plus));
	Button_minus.signal_clicked().connect( sigc::mem_fun(*this, &Window::on_button_minus));
	Button_submit.signal_clicked().connect( sigc::mem_fun(*this, &Window::on_button_submit));
	Button_reset.signal_clicked().connect( sigc::mem_fun(*this, &Window::on_button_reset));

	// Focus controller
	// the signal is emitted, when we select another window application for example
	auto focus_controller = Gtk::EventControllerFocus::create();
	this->add_controller(focus_controller);
	focus_controller->signal_leave().connect([this]() {
		printf("logbook-gui lost the focus.\n");
		if (flag_screenshot) {
			if (Notebook_screenshots.get_n_pages() > 0) {
				printf("screenshot ongoing\n");
				std::time_t now = std::time(nullptr);
				// files dir
				std::string files_dir = output_dir + "/files/" + time_t2string(now, "%Y/%m/%d");  
				std::string filename  = files_dir + "/" + "screenshot_" + time_t2string(now, "%Y-%m-%d_%H-%M-%S") + ".png";
				std::filesystem::create_directories(files_dir.c_str());
				std::string command = "./screenshot.sh " + filename;
				std::system(command.c_str());
				// il ne reste plus récupérer le ficher et à l'afficher dans le Notebook. 
				// idée, récupérer le numéro de page actif, insérer une Gtk::Image, etc...
				// jouer avec les boutons + et -
				// Gtk::Notebook
				int page_number = Notebook_screenshots.get_current_page();
				printf("n pages : %d\n", Notebook_screenshots.get_n_pages());
				attachments.at(page_number) = filename;
				update_screenshots();
				Notebook_screenshots.set_current_page(page_number);
			}
			flag_screenshot = false;
		}
        });
	focus_controller->signal_enter().connect([this]() {
		printf("logbook-gui has the focus.\n");
        });

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

void Window::on_button_take_screenshot() {
	if (Notebook_screenshots.get_n_pages() <= 0) {
		printf("create a new page with +...\n");
		Label_log.set_text("Please create a new page with +");
		return;
	}
	printf("Action: take screenshot...\n");
	flag_screenshot = true;
	// css style
	auto context = Label_log.get_style_context();
	context->remove_class("label-log1");
	context->remove_class("label-log2");
	context->add_class("label-log1");
	// end css style
	Label_log.set_text("New screenshot...");
}

void Window::on_button_plus() {
	printf("Action: take screenshot / plus...\n");
	attachments.push_back("");
	Notebook_screenshots.append_page(*Gtk::make_managed<Gtk::Picture>(), "empty");
	Notebook_screenshots.set_current_page(Notebook_screenshots.get_n_pages()-1);
	// css style
	auto context = Label_log.get_style_context();
	context->remove_class("label-log1");
	context->remove_class("label-log2");
	context->add_class("label-log1");
	// end css style
	Label_log.set_text("Add a new screenshot tab...");
}

void Window::on_button_minus() {
	printf("Action: take screenshot / minus...\n");
	int page_number = Notebook_screenshots.get_current_page();
	if ((page_number >= 0) && (page_number < (int) attachments.size())) {
		attachments.erase(attachments.begin() + page_number);
	}
	update_screenshots();
	// css style
	auto context = Label_log.get_style_context();
	context->remove_class("label-log1");
	context->remove_class("label-log2");
	context->add_class("label-log1");
	// end css style
	Label_log.set_text("Remove a screenshot tab...");
}

void Window::on_button_submit() {
	printf("Action: submit...\n");
	std::time_t now = std::time(nullptr);
	// css style
	auto context = Label_log.get_style_context();
	context->remove_class("label-log1");
	context->remove_class("label-log2");
	context->add_class("label-log2");
	// end css style
	// title    
	std::string title = Entry_title.get_text();
	std::regex empty_string(R"(^\s*$)"); // \s* zéro ou plusieurs caractères blancs ( , \t, \n, etc.)
	if (std::regex_match(title, empty_string) || title == std::string("Enter a title for log entry...")) {
		printf("*Please provide a valid title\n");
		Label_log.set_text("Invalid title...");
		return;
	}
	// other entries
	std::string marker = Entry_marker.get_text();
	std::string email = Entry_email.get_text();
	std::string comments = TextBuffer_comments->get_text();
	printf("===================================\n");
	printf("Entry submited on : %s\n", std::ctime(&now));
	printf("Title: %s\n", title.c_str());
	printf("   marker  : %s\n", marker.c_str());
	printf("   send to : %s\n\n", email.c_str());
	printf("%s\n", comments.c_str());
	printf("Attachments : \n");
	int num = 0;
	for (std::string filename : attachments) {
		if (filename != "") {
			num++;
			printf("   %d. %s\n", num, filename.c_str());
		}
	}
	printf("===================================\n");
	Label_log.set_text("Log entry submitted...");
	// file output
	std::string entry_dir  = output_dir + "/entry/" + time_t2string(now, "%Y/%m/%d");  
	std::string entry_file = entry_dir + "/" + time_t2string(now, "%Y-%m-%d_%H-%M-%S") + ".md";
	std::filesystem::create_directories(entry_dir.c_str());
	std::ofstream file(entry_file.c_str(), std::ios::out);
	if (!file.is_open()) {
        	return; 
    	}
	file << "## "  << title << std::endl << std::endl;
	file << "Submitted on " << time_t2string(now, "%a, %d %b %Y - %H:%M:%S %Z") << std::endl << std::endl;
	file << "   _marker_ :" << marker << std::endl << std::endl;
	file << "   _email_ :" << email << std::endl << std::endl;
	file << comments << std::endl << std::endl;
	file << "**Attachments:**" << std::endl << std::endl;
	int natt = 0;
	for (std::string att : attachments) {
		if (att != "") {
			natt++;
			file << natt << ". ![" << att << "](../../../../" << att << ")  " << std::endl;
		}
	}
	file.close();
	
	// save entry in the database (chatGPT of course!)
		// open a sqlite db
	sqlite3* db;
	char* errMsg = nullptr;
	int rc = sqlite3_open("./data.db", &db);
	if (rc) {
		std::cerr << "Cannot open the database " << sqlite3_errmsg(db) << "\n";
		return;
	}
		// prepare the command
	std::string insertSQL = "INSERT INTO entries (timestamp, author, title) VALUES ('" + time_t2string(now, "%Y-%m-%d %H:%M:%S") + "', '" + "touchte" + "', '" + title + "');";
		// execute the command
	if (sqlite3_exec(db, insertSQL.c_str(), nullptr, nullptr, &errMsg) != SQLITE_OK) {
		std::cerr << "Erreur insertion: " << errMsg << std::endl;
	sqlite3_free(errMsg);
	} else {
		std::cout << "New entry added in the database" << std::endl;
	}
	sqlite3_close(db);
}

void Window::on_button_reset() {
	printf("Action: reset...\n");
	Entry_title.set_text("Enter a title for log entry...");
	Entry_email.set_text("");
	Entry_marker.set_text("");
	Label_log.set_text("");
	TextBuffer_comments->set_text("");
	attachments.clear();
	update_screenshots();
}

void Window::update_screenshots() {
	// empty the notebook
	for (int i = Notebook_screenshots.get_n_pages()-1; i >= 0; i--){
		Notebook_screenshots.remove_page(i);
	}
	// fill it back
	int page_number = 0;
	for (std::string filename : attachments) {
		page_number++;
		char page_name[50];
		if (filename != "") {
			sprintf(page_name, "#%d", page_number);
		}
		else {
			sprintf(page_name, "empty");
		}
		Notebook_screenshots.append_page(*Gtk::make_managed<Gtk::Picture>(filename.c_str()), page_name);
	}
}

std::string Window::time_t2string(std::time_t t, std::string format) {
	std::tm tm = *std::localtime(&t);
	std::ostringstream oss;
	oss << std::put_time(&tm, format.c_str());
	return oss.str();
}

int Window::get_last_lognumber() {
	int last_lognumber = -1;
	// Access last lognumber (chatGPT)
		// open db
	sqlite3* db;
	sqlite3_stmt* stmt; // pour préparer et exécuter la requête
	if (sqlite3_open("data.db", &db) != SQLITE_OK) {
        	std::cerr << "Erreur ouverture DB: " << sqlite3_errmsg(db) << std::endl;
        	return -1;
	}
		// préparation de la requête
	const char* sql = "SELECT MAX(lognumber) FROM entries;";
	if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        	std::cerr << "Erreur préparation requête: " << sqlite3_errmsg(db) << std::endl;
        	sqlite3_close(db);
        	return -1;
	}
		// exécution de la requête
	int rc = sqlite3_step(stmt);
		// Si une ligne de résultat est disponible, on rentre dans ce bloc.
		// On lit la première (et unique) colonne de cette ligne (index 0).// Si la colonne vaut NULL (exemple : table vide), on met max_id à 0. // Sinon, on récupère la valeur entière de cette colonne dans max_id.
	if (rc == SQLITE_ROW) {
		last_lognumber = sqlite3_column_type(stmt, 0) == SQLITE_NULL ? 0 : sqlite3_column_int(stmt, 0);
		std::cout << "ID max dans la table: " << last_lognumber << std::endl;
	} else {
		std::cerr << "Erreur lecture résultat" << std::endl;
	}
		// Libère la mémoire allouée pour la requête préparée stmt.
		// close db
	sqlite3_finalize(stmt);
	sqlite3_close(db);
	return last_lognumber;
}

/** Main function */
int main (int argc, char * argv[]) {
	printf("Start logbook-gui...\n");

	auto app = Gtk::Application::create("org.gtkmm.example.logbook");

	return app->make_window_and_run<Window>(argc, argv);
}



