#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

#include "package.h"

#include "lib/nsm.h"

#include "core/midifile.h"
#include "core/configfile.h"
#include "core/cachefile.h"
#include "core/perform.h"

#include "gui/mainwindow.h"
#include <gtkmm.h>

/* struct for command parsing */
static struct
option long_options[] = {

    {"file",     required_argument, 0, 'f'},
    {"config", 1,0,'c'},
    {"help",     0, 0, 'h'},
    {"osc-port", 1,0,'p'},
    {"jack-transport",0, 0, 'j'},
    {"no-gui",0, 0, 'n'},
    {"version",0, 0, 'v'},
    {0, 0, 0, 0}

};

string config_filename = "";
string global_filename = "";
string last_used_dir = getenv("HOME");

bool global_no_gui = false;
bool global_with_jack_transport = false;

bool global_is_running = true;

char* global_oscport;

string global_client_name = PACKAGE;

user_midi_bus_definition   global_user_midi_bus_definitions[c_maxBuses];
user_instrument_definition global_user_instrument_definitions[c_max_instruments];
user_keymap_definition     global_user_keymap_definitions[c_max_instruments];

Glib::RefPtr<Gtk::Application> application;

// nsm
bool global_nsm_gui = false;
bool nsm_opional_gui_support = true;
nsm_client_t *nsm = 0;
bool nsm_wait = true;
string nsm_folder = "";

int nsm_save_cb(char **,  void *userdata);

void nsm_hide_cb(void *userdata);

void nsm_show_cb(void *userdata);

int nsm_open_cb(const char *name, const char *display_name, const char *client_id, char **out_msg, void *userdata);

int librun();