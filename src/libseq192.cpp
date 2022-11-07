// This file is part of seq192
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
#include "libseq192.hpp"


int
nsm_save_cb(char **,  void *userdata)
{
    MainWindow *w = (MainWindow *) userdata;
    w->nsm_save();
    return ERR_OK;
}
void
nsm_hide_cb(void *userdata)
{
    application->hold();
    global_nsm_gui = false;
}
void
nsm_show_cb(void *userdata)
{
    global_nsm_gui = true;
}
int
nsm_open_cb(const char *name, const char *display_name, const char *client_id, char **out_msg, void *userdata)
{
    nsm_wait = false;
    nsm_folder = name;
    global_client_name = client_id;
    // NSM API 1.1.0: check if server supports optional-gui
    nsm_opional_gui_support = strstr(nsm_get_session_manager_features(nsm), "optional-gui");
    mkdir(nsm_folder.c_str(), 0777);
    // make sure nsm server doesn't override cached visibility state
    nsm_send_is_shown(nsm);
    return ERR_OK;
}


int
librun ()
{

    for (int i=0; i<c_maxBuses; i++)
    {
        for (int j=0; j<16; j++) {
            global_user_midi_bus_definitions[i].instrument[j] = -1;
            global_user_midi_bus_definitions[i].keymap[j] = -1;
        }
    }

    for (int i=0; i<c_max_instruments; i++)
    {
        for (int j=0; j<128; j++)
            global_user_instrument_definitions[i].controllers_active[j] = false;
    }

    for (int i=0; i<c_max_instruments; i++)
    {
        for (int j=0; j<128; j++)
            global_user_keymap_definitions[i].keys_active[j] = false;
    }

    // nsm
    const char *nsm_url = getenv( "NSM_URL" );
    if (nsm_url) {
        nsm = nsm_new();
        nsm_set_open_callback(nsm, nsm_open_cb, 0);
        if (nsm_init(nsm, nsm_url) == 0) {
            nsm_send_announce(nsm, PACKAGE, ":optional-gui:dirty:", "libseq192");
        }
        int timeout = 0;
        while (nsm_wait) {
            nsm_check_wait(nsm, 500);
            timeout += 1;
            if (timeout > 200) exit(1);
        }
    }

    /* the main performance object */
    perform * p = new perform();

    // read config file
    string config_path = getenv("XDG_CONFIG_HOME") == NULL ? string(getenv("HOME")) + "/.config" : getenv("XDG_CONFIG_HOME");
    config_path += string("/") + PACKAGE;
    mkdir(config_path.c_str(), 0777);
    if (nsm) config_path = nsm_folder;
    string file_path = config_filename == "" ? (config_path + "/config.json") : config_filename;
    std::ifstream infile(file_path);
    if (!infile.good()) {
        std::fstream fs;
        fs.open(file_path, std::ios::out);
        fs << "{}" << endl;
        fs.close();
    }
    ConfigFile config(file_path);
    config.parse();

    // read/touch cache file
    string cache_path = getenv("XDG_CACHE_HOME") == NULL ? string(getenv("HOME")) + "/.cache" : getenv("XDG_CACHE_HOME");
    cache_path += string("/") + PACKAGE;
    mkdir(cache_path.c_str(), 0777);
    if (nsm) cache_path = nsm_folder;
    file_path = cache_path + "/cache.json";
    infile = std::ifstream(file_path);
    if (!infile.good()) {
        std::fstream fs;
        fs.open(file_path, std::ios::out);
        fs << "{}" << endl;
        fs.close();
    }
    CacheFile cache(file_path);
    cache.parse();

    p->init();

    p->launch_input_thread();
    p->launch_output_thread();
    p->init_jack();

    if (nsm) {
        global_filename = nsm_folder + "/session.midi";
        // write session file if it doesn't exist
        std::ifstream infile(global_filename);
        if (!infile.good()) {
            midifile f(global_filename);
            f.write(p, -1, -1);
        }
    }

    if (global_filename != "") {
        midifile *f = new midifile(global_filename);
        f->parse(p, 0);
        delete f;
    }

    if (global_oscport != 0 && strstr(global_oscport, "/")) {
        // liblo needs a gracefull ctrl+c handler to release unix socket
        signal(SIGINT, [](int param){global_is_running = false;});
    }

    int status = 0;
    if (global_no_gui) {
        while (global_is_running) {
            usleep(1000);
        }
    } else {
        application = Gtk::Application::create();
        MainWindow window(p, application);

        if (nsm) {
            // register callbacks
            nsm_set_save_callback(nsm, nsm_save_cb, (void*) &window);
            // setup optional-gui
            if (nsm_opional_gui_support) {
                nsm_set_show_callback(nsm, nsm_show_cb, 0);
                nsm_set_hide_callback(nsm, nsm_hide_cb, 0);
                if (!global_nsm_gui) nsm_hide_cb(0);
                else nsm_send_is_shown(nsm);
            } else {
                global_nsm_gui = true;
            }
            // enable nsm in window
            window.nsm_set_client(nsm, nsm_opional_gui_support);
            // bind quit signal
            signal(SIGTERM, [](int param){
                global_is_running = false;
                application->quit();
            });
        }

        // bind ctrl+c signal
        signal(SIGINT, [](int param){
            global_is_running = false;
            application->quit();
        });


        status = application->run(window);
    }

    // write cache file
    cache.write();

    delete p;

    if (nsm) {
        nsm_free(nsm);
        nsm = NULL;
    }

    return status;
}
