#include "window.hpp"
#include "main.hpp"

#include <thread>
#include <iostream>
#include <cstdlib>
#include <string>
#include <iomanip>
#include <gtk4-layer-shell.h>

std::string action_type;

void thread() {
	char command[20] = "";

	// Figure out what we're doing
	if (action_type == "shutdown") {
		strcpy(command, "loginctl poweroff");
		win->label_status.set_label("Shutting down...");
	}
	else if (action_type == "reboot") {
		strcpy(command, "loginctl reboot");
		win->label_status.set_label("Rebooting...");
	}
	else if (action_type == "logout") {
		strcpy(command, "loginctl logout");
		win->label_status.set_label("Logging out...");
	}

	// TODO: Give a visual indicator that stuff is going on, Add a spinner.
	win->box_buttons.set_visible(false);
	win->progressbar_sync.set_visible(true);
	win->max_slider_value = get_dirty_pages();

	// Start killing children (Scary)
	// For now this only prints them (Boring)
	print_child_processes();

	// Sync filesystems
	win->timer_connection = Glib::signal_timeout().connect(sigc::mem_fun(*win, &syspower::on_timer_tick), 50);
	win->progressbar_sync.set_fraction(1);
	sync_filesystems();

	// Cleanup
	win->progressbar_sync.set_fraction(0);
	win->timer_connection.disconnect();

	// Run action
	system(command);
}

syspower::syspower() {
	gtk_layer_init_for_window(gobj());
	gtk_layer_set_namespace(gobj(), "syspower");
	gtk_layer_set_layer(gobj(), GTK_LAYER_SHELL_LAYER_TOP);

	// TODO: Add customization, For now assume fullscreen.
	gtk_layer_set_anchor(gobj(), GTK_LAYER_SHELL_EDGE_LEFT, true);
	gtk_layer_set_anchor(gobj(), GTK_LAYER_SHELL_EDGE_RIGHT, true);
	gtk_layer_set_anchor(gobj(), GTK_LAYER_SHELL_EDGE_TOP, true);
	gtk_layer_set_anchor(gobj(), GTK_LAYER_SHELL_EDGE_BOTTOM, true);

	// Set window title
	set_title("Power Menu");

	// Set window size
	set_default_size(200, 100);

	// Add the button to the window
	set_child(box_layout);

	// TODO: Add customization options for this.
	box_layout.property_orientation().set_value(Gtk::Orientation::VERTICAL);
	box_buttons.property_orientation().set_value(Gtk::Orientation::VERTICAL);
	box_layout.set_valign(Gtk::Align::CENTER);
	box_layout.set_halign(Gtk::Align::CENTER);

	box_layout.append(label_status);
	box_layout.append(box_buttons);
	box_layout.append(progressbar_sync);
	box_buttons.set_halign(Gtk::Align::CENTER);

	// TODO: Idk but i feel like adding a revealer to box_buttons would make 
	// showing the bar a little fancier?
	label_status.set_margin(10);
	progressbar_sync.set_size_request(300, -1);
	progressbar_sync.set_visible(false);

	// Button shenanigans
	box_buttons.append(button_shutdown);
	button_shutdown.set_size_request(300, 75);
	button_shutdown.set_label("Shutdown");
	button_shutdown.set_margin(5);

	box_buttons.append(button_reboot);
	button_reboot.set_size_request(300, 75);
	button_reboot.set_label("Reboot");
	button_reboot.set_margin(5);

	box_buttons.append(button_logout);
	button_logout.set_size_request(300, 75);
	button_logout.set_label("Logout");
	button_logout.set_margin(5);

	box_buttons.append(button_cancel);
	button_cancel.set_size_request(300, 75);
	button_cancel.set_label("Cancel");
	button_cancel.set_margin(5);

	// Signals
	button_shutdown.signal_clicked().connect(sigc::mem_fun(*this, &syspower::button_shutdown_clicked));
	button_reboot.signal_clicked().connect(sigc::mem_fun(*this, &syspower::button_reboot_clicked));
	button_logout.signal_clicked().connect(sigc::mem_fun(*this, &syspower::button_logout_clicked));
	button_cancel.signal_clicked().connect(sigc::mem_fun(*this, &syspower::button_cancel_clicked));

	show();
}

// TODO: Fix this horrible mess, learn to use dynamic casting or something idk.
void syspower::button_shutdown_clicked() {
	action_type = "shutdown";
	thread_action = std::thread(thread);
}

void syspower::button_reboot_clicked() {
	action_type = "reboot";
	thread_action = std::thread(thread);
}

void syspower::button_logout_clicked() {
	action_type = "logout";
	thread_action = std::thread(thread);
}

void syspower::button_cancel_clicked() {
	app->quit();
}

bool syspower::on_timer_tick() {
	double dirty_pages_kb = get_dirty_pages();
	double value = dirty_pages_kb / max_slider_value;

	if (progressbar_sync.get_fraction() > value)
		progressbar_sync.set_fraction(value);

	return true;
}
