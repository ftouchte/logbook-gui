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
				int entry_number = get_last_lognumber() + 1;
				std::string files_dir = "files/" + time_t2string(now, "%Y/%m/%d") + "/" + std::to_string(entry_number);  
				std::string filename  = files_dir + "/" + "screenshot_" + time_t2string(now, "%Y-%m-%d_%H-%M-%S") + ".png";
				std::filesystem::create_directories((output_dir + "/" + files_dir).c_str());
				std::string command = output_dir + "/screenshot.sh " + output_dir + "/" + filename;
				if (std::system(command.c_str()) == 0) {
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
	m_refCssProvider->load_from_path(std::string(output_dir + "/custom_gtkmm.css").c_str());
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
	int entry_number = get_last_lognumber() + 1;
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
	std::string entry_dir  = output_dir + "/entry/" + std::to_string(entry_number);  
	std::string entry_file = entry_dir + "/" + "index.html";
	std::filesystem::create_directories(entry_dir.c_str());
	std::ofstream file(entry_file.c_str(), std::ios::out);
	if (!file.is_open()) {
        	return; 
    	}
	file << "<!DOCTYPE html>" << std::endl;
	file << "<html>" << std::endl;
	file << "<head>" << std::endl;
	file << "  <meta charset='UTF-8'>" << std::endl;
	file << "  <title>ftouchte | Log Entry</title>" << std::endl;
	file << "  <link rel='stylesheet' href='../../stylesheet_entry.css'>" << std::endl;
	file << "</head>" << std::endl;
	file << "<body>" << std::endl;
	file << "<div id=header>" << std::endl;
        file << "   <h1> <a href='https://ftouchte.github.io/logbook-gui/'> Felix Touchte Codjo - Electronic Logbook </a> </h1>" << std::endl;
        file << "</div>" << std::endl;
	file << "<div id=pagination>" << std::endl;
	file << "<button onclick='Prev()'> Prev </button> <button onclick='Next()'> Next </button>" << std::endl;
	file << "</div>" << std::endl;
	file << "<h2> "  << title << "</h2>" << std::endl;
	file << "<p> Lognumber <span id='lognumber'>"  << entry_number << "</span>. </p>";
	file << "<p> Submitted on " << time_t2string(now, "%a, %d %b %Y - %H:%M:%S %Z") << "</p>" << std::endl;
	file << "<p> <i> Marker: </i>" << marker << "</p>" << std::endl;
	file << "<p> <i> Send to: </i>" << email << "</p>" << std::endl;
	file << "<div>" << std::endl;
	file << comments << std::endl;
	file << "</div>" << std::endl;
	file << "<p>" << std::endl;
	int natt = 0;
	for (std::string att : attachments) {
		if (att != "") {
			natt++;
		}
	}
	file << "   <b> Attachments (" << natt << ") </b>" << std::endl;
	file << "</p>" << std::endl;
	natt = 0;
	for (std::string att : attachments) {
		if (att != "") {
			natt++;
			file << "<p> " << natt << ". </p>" << std::endl;
			file << "<p> <img src='../../"<< att << "' alt='" << att << "'> </p>" << std::endl;
		}
	}
	file << "</body>" << std::endl;
	file << "<script src='../../script_entry.js'> </script>" << std::endl;	
	file << "</html>" << std::endl;
	file.close();
	
	// save entry in the database (chatGPT of course!)
		// open a sqlite db
	sqlite3* db;
	std::string db_name = output_dir + "/data.db";
	char* errMsg = nullptr;
	int rc = sqlite3_open(db_name.c_str(), &db);
	if (rc) {
		std::cerr << "Cannot open the database " << sqlite3_errmsg(db) << "\n";
		return;
	}
		// prepare the command
	std::string insertSQL = "INSERT INTO entries (date, time, author, title) VALUES ('" + time_t2string(now, "%Y-%m-%d") + "', '" + time_t2string(now, "%H:%M:%S") + "', '" + "touchte" + "', '" + title + "');";
		// execute the command
	if (sqlite3_exec(db, insertSQL.c_str(), nullptr, nullptr, &errMsg) != SQLITE_OK) {
		std::cerr << "Erreur insertion: " << errMsg << std::endl;
	sqlite3_free(errMsg);
	} else {
		std::cout << "New entry added in the database" << std::endl;
	}
	sqlite3_close(db);
	// deploy logbook
	deploy_logbook();
}

void Window::on_button_reset() {
	printf("Action: reset...\n");
	int entry_number = get_last_lognumber() + 1;
	char buffer[100];
	sprintf(buffer, "Entry n° %d: Enter a title for your log entry...", entry_number);
	Entry_title.set_text(buffer);
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
		Notebook_screenshots.append_page(*Gtk::make_managed<Gtk::Picture>((output_dir + "/" + filename).c_str()), page_name);
	}
}

std::string Window::time_t2string(std::time_t t, std::string format) {
	std::tm tm = *std::localtime(&t);
	std::ostringstream oss;
	oss << std::put_time(&tm, format.c_str());
	return oss.str();
}

time_t Window::string2time_t(std::string date, std::string format) {
	std::tm tm = {};
	std::istringstream iss(date);
	iss >> std::get_time(&tm, format.c_str());
	if (iss.fail()) {
		throw std::runtime_error("Invalid format for date");
	}
	return std::mktime(&tm);
}

int Window::get_last_lognumber() {
	int last_lognumber = -1;
	// Access last lognumber (chatGPT)
		// open db
	sqlite3* db;
	std::string db_name = output_dir + "/data.db";
	sqlite3_stmt* stmt; // pour préparer et exécuter la requête
	if (sqlite3_open(db_name.c_str(), &db) != SQLITE_OK) {
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

void Window::deploy_logbook() {
	std::vector<std::string> all_dates;
	{ // extract all distinct dates from db
		sqlite3* db;
		sqlite3_stmt* stmt;
		std::string db_name = output_dir + "/data.db";
		const char* sql = "SELECT DISTINCT date FROM entries ORDER BY date DESC;"; // Exemple de table

		// Ouvrir la base de données
		if (sqlite3_open(db_name.c_str(), &db) != SQLITE_OK) {
			std::cerr << "Erreur ouverture DB: " << sqlite3_errmsg(db) << std::endl;
			return;
		}

		// Préparer la requête
		if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
			std::cerr << "Erreur préparation requête: " << sqlite3_errmsg(db) << std::endl;
			sqlite3_close(db);
			return;
		}
		// Lire les lignes
		printf("All distinct dates : ");
		while (sqlite3_step(stmt) == SQLITE_ROW) {
			const unsigned char* date = sqlite3_column_text(stmt, 0);
			printf("%s, ", date);
			all_dates.push_back(reinterpret_cast<const char *>(date));
		}
		printf("\n");
		// Libérer les ressources
		sqlite3_finalize(stmt);
		sqlite3_close(db);
	}
	// create a file index.html (the homepage)
	std::string db_name = output_dir + "/data.db";
	std::ofstream file(std::string(output_dir + "/index.html").c_str(), std::ios::out);
	if (!file.is_open()) {
		printf("cannot open index.html\n");
		return; 
	}
	file << "<!DOCTYPE html>" << std::endl;
	file << "<html>" << std::endl;
	file << "<head>" << std::endl;
	file << "  <meta charset='UTF-8'>" << std::endl;
	file << "  <title>ftouchte | Logbook</title>" << std::endl;
	file << "  <link rel='stylesheet' href='./stylesheet_homepage.css'>" << std::endl;
	file << "</head>" << std::endl;
		// div : header
	file << "<body>" << std::endl;
	file << "<div id=header>" << std::endl;
	file << "   <h1> <a href='https://ftouchte.github.io/logbook-gui/'> Felix Touchte Codjo - Electronic Logbook </a> </h1>" << std::endl;
	file << "</div>" << std::endl;
	// div : main
	file << "<div id=main>" << std::endl;
	for (std::string this_date : all_dates)
	{ // start loop over date 
		sqlite3* db;
		sqlite3_stmt* stmt;
		std::string db_name = output_dir + "/data.db";
		std::string sql = "SELECT lognumber, date, time, author, title FROM entries WHERE date = ? ORDER BY lognumber DESC;"; // Exemple de table

		// Ouvrir la base de données
		if (sqlite3_open(db_name.c_str(), &db) != SQLITE_OK) {
			std::cerr << "Erreur ouverture DB: " << sqlite3_errmsg(db) << std::endl;
			return;
		}

		// Préparer la requête
		if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
			std::cerr << "Erreur préparation requête: " << sqlite3_errmsg(db) << std::endl;
			sqlite3_close(db);
			return;
		}
		sqlite3_bind_text(stmt, 1, this_date.c_str(), -1, SQLITE_STATIC);
		// this_date
		file << "<p> <b>" << time_t2string(string2time_t(this_date, "%Y-%m-%d"), "%A, %d %B %Y") << " </b> </p>" << std::endl;	
		file << "   <table>" << std::endl;
		file << "      <tr>" << std::endl;
		file << "         <th>" << "Lognumber" << "</th>" << std::endl;
		file << "         <th>" << "Date" << "</th>" << std::endl;
		file << "         <th>" << "Author" << "</th>" << std::endl;
		file << "         <th>" << "Title" << "</th>" << std::endl;
		file << "      </tr>" << std::endl;
		// Lire les lignes
		while (sqlite3_step(stmt) == SQLITE_ROW) {
			int lognumber = sqlite3_column_int(stmt, 0);
			const unsigned char* date = sqlite3_column_text(stmt, 1);
			const unsigned char* time = sqlite3_column_text(stmt, 2);
			const unsigned char* author = sqlite3_column_text(stmt, 3);
			const unsigned char* title = sqlite3_column_text(stmt, 4);
			printf("%d | %s | %s | %s | %s\n", lognumber, date, time, author, title);
			file << "      <tr>" << std::endl;
			//file << "         <td>" << lognumber << "</td>" << std::endl;
			file << "         <td>" << "<a href='./entry/" << lognumber << "/index.html'>" << lognumber << "<a>" << "</td>" << std::endl;
			file << "         <td>" << date << " - " << time << "</td>" << std::endl;
			file << "         <td>" << author << "</td>" << std::endl;
			file << "         <td>" << "<a href='./entry/" << lognumber << "/index.html'>" << title << "<a>" << "</td>" << std::endl;
			file << "      </tr>" << std::endl;
			
		}
		file << "   <table>" << std::endl;
		// Libérer les ressources
		sqlite3_finalize(stmt);
		sqlite3_close(db);
	} // end loop over date
	file << "</div>" << std::endl;
	file << "</body>" << std::endl;
	file << "</html>" << std::endl;
	file.close();
	// create the script file : script_entry.js
	std::ofstream file2(std::string(output_dir + "/script_entry.js").c_str(), std::ios::out);
	if (!file2.is_open()) {
		printf("cannot open script_entry.js\n");
		return; 
	}
	file2 << " const lognumbers = [";
	std::vector<int> vec_lognumbers = get_lognumbers();
	for (int i = 0; i < (int) vec_lognumbers.size(); i++) {
		file2 << vec_lognumbers[i];
		if (i < (int) vec_lognumbers.size() - 1) {
			file2 << ", ";
		}
	}
	file2 << "];" << std::endl;
	file2 << "  " << std::endl;
	file2 << " function Next() { " << std::endl;
	file2 << " 	const text_lognumber = document.getElementById('lognumber').textContent; " << std::endl;
	file2 << " 	const lognumber = Number(text_lognumber); " << std::endl;
	file2 << " 	let next_lognumber = lognumbers[0]; " << std::endl;
	file2 << " 	for (let i = 0; i < lognumbers.length - 1; i++) { " << std::endl;
	file2 << " 		if (lognumbers[i] === lognumber) { " << std::endl;
	file2 << " 			next_lognumber = lognumbers[i+1]; " << std::endl;
	file2 << " 			break; " << std::endl;
	file2 << " 		} " << std::endl;
	file2 << " 	} " << std::endl;
	file2 << " 	const page = '../' + next_lognumber + '/index.html'; " << std::endl;
	file2 << " 	window.location.href = page; " << std::endl;
	file2 << " } " << std::endl;
	file2 << "  " << std::endl;
	file2 << " function Prev() { " << std::endl;
	file2 << " 	const text_lognumber = document.getElementById('lognumber').textContent; " << std::endl;
	file2 << " 	const lognumber = Number(text_lognumber); " << std::endl;
	file2 << " 	let prev_lognumber = lognumbers[0]; " << std::endl;
	file2 << " 	for (let i = 1; i < lognumbers.length; i++) { " << std::endl;
	file2 << " 		if (lognumbers[i] === lognumber) { " << std::endl;
	file2 << " 			prev_lognumber = lognumbers[i-1]; " << std::endl;
	file2 << " 			break; " << std::endl;
	file2 << " 		} " << std::endl;
	file2 << " 	} " << std::endl;
	file2 << " 	const page = '../' + prev_lognumber + '/index.html'; " << std::endl;
	file2 << " 	window.location.href = page; " << std::endl;
	file2 << " } " << std::endl;
	file2.close();
}

std::vector<int> Window::get_lognumbers() {
	std::vector<int> vec_lognumbers;
	// extract all lognumber
	sqlite3* db;
	sqlite3_stmt* stmt;
	std::string db_name = output_dir + "/data.db";
	const char* sql = "SELECT lognumber FROM entries ORDER BY lognumber ASC;"; // Exemple de table

	// Ouvrir la base de données
	if (sqlite3_open(db_name.c_str(), &db) != SQLITE_OK) {
		std::cerr << "Erreur ouverture DB: " << sqlite3_errmsg(db) << std::endl;
		return vec_lognumbers;
	}

	// Préparer la requête
	if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
		std::cerr << "Erreur préparation requête: " << sqlite3_errmsg(db) << std::endl;
		sqlite3_close(db);
		return vec_lognumbers;
	}
	// Lire les lignes
	printf("All available lognumber : ");
	while (sqlite3_step(stmt) == SQLITE_ROW) {
		int lognumber = sqlite3_column_int(stmt, 0);
		printf("%d, ", lognumber);
		vec_lognumbers.push_back(lognumber);
	}
	printf("\n");
	// Libérer les ressources
	sqlite3_finalize(stmt);
	sqlite3_close(db);
	return vec_lognumbers;
}

/** Main function */
int main (int argc, char * argv[]) {
	printf("Start logbook-gui...\n");
	if (argc > 1) {
		if (std::string(argv[1]) == std::string("test")) {
			if (!std::system("python3 -m http.server 8080 --directory /home/touchte-codjo/.gui-logbook")) { return -1;};
			return 0;
		}
		else if (std::string(argv[1]) == std::string("deploy")) {
			if (!std::system("here=$(pwd); cd /home/touchte-codjo/.gui-logbook; git add .; git commit -m \"deploy gui-logbook\"; git push; cd $here;")) { return -1;};
			return 0;
		}
	}

	auto app = Gtk::Application::create("org.gtkmm.example.logbook");

	return app->make_window_and_run<Window>(argc, argv);
}



