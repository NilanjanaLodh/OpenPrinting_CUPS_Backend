#include <stdio.h>
#include <stdlib.h>
#include <glib.h>
#include <string.h>

#include "CPD.h"

void display_help();
gpointer parse_commands(gpointer user_data);
FrontendObj *f;

static int add_printer_callback(PrinterObj *p)
{
    printf("print_frontend.c : Printer %s added!\n", p->name);
}

static int remove_printer_callback(char *printer_name)
{
    printf("print_frontend.c : Printer %s removed!\n", printer_name);
}

int main(int argc, char **argv)
{
    event_callback add_cb = (event_callback)add_printer_callback;
    event_callback rem_cb = (event_callback)remove_printer_callback;

    char *dialog_bus_name = malloc(300);
    if (argc > 1) //this is for creating multiple instances of a dialog simultaneously
        f = get_new_FrontendObj(argv[1], add_cb, rem_cb);
    else
        f = get_new_FrontendObj(NULL, add_cb, rem_cb);

    g_thread_new("parse_commands_thread", parse_commands, NULL);
    connect_to_dbus(f);
    GMainLoop *loop = g_main_loop_new(NULL, FALSE);
    g_main_loop_run(loop);
}

gpointer parse_commands(gpointer user_data)
{
    printf("parse_commands\n");
    fflush(stdout);
    char buf[100];
    while (1)
    {
        printf("> ");
        fflush(stdout);
        scanf("%s", buf);
        if (strcmp(buf, "stop") == 0)
        {
            disconnect_from_dbus(f);
            g_message("Stopping front end..\n");
            exit(0);
        }
        else if (strcmp(buf, "refresh") == 0)
        {
            refresh_printer_list(f);
            g_message("Getting changes in printer list..\n");
        }
        else if (strcmp(buf, "hide-remote-cups") == 0)
        {
            hide_remote_cups_printers(f);
            g_message("Hiding remote printers discovered by the cups backend..\n");
        }
        else if (strcmp(buf, "unhide-remote-cups") == 0)
        {
            unhide_remote_cups_printers(f);
            g_message("Unhiding remote printers discovered by the cups backend..\n");
        }
        else if (strcmp(buf, "hide-temporary-cups") == 0)
        {
            hide_temporary_cups_printers(f);
            g_message("Hiding remote printers discovered by the cups backend..\n");
        }
        else if (strcmp(buf, "unhide-temporary-cups") == 0)
        {
            unhide_temporary_cups_printers(f);
            g_message("Unhiding remote printers discovered by the cups backend..\n");
        }
        // else if (strcmp(buf, "update-basic") == 0)
        // {
        //     char printer_name[100];
        //     scanf("%s", printer_name);
        //     g_message("Updating basic options ..\n");
        //     update_basic_printer_options(f, printer_name);
        // }
        // else if (strcmp(buf, "get-capabilities") == 0)
        // {
        //     char printer_name[100];
        //     scanf("%s", printer_name);
        //     g_message("Getting basic capabilities ..\n");
        //     get_printer_capabilities(f, printer_name);
        // }
        else if (strcmp(buf, "get-all-options") == 0)
        {
            char printer_name[100];
            char backend_name[100];
            scanf("%s%s", printer_name, backend_name);
            g_message("Getting all attributes ..\n");
            get_all_printer_options(f, printer_name, backend_name);
        }
        // else if (strcmp(buf, "get-option-default") == 0)
        // {
        //     char printer_name[100], option_name[100];
        //     scanf("%s%s", printer_name, option_name);
        //     get_printer_option_default(f, printer_name, option_name);
        // }
        // else if (strcmp(buf, "get-supported-raw") == 0)
        // {
        //     char printer_name[100], option_name[100];
        //     scanf("%s%s", printer_name, option_name);
        //     get_printer_supported_values_raw(f, printer_name, option_name);
        // }
        // else if (strcmp(buf, "get-supported-media") == 0)
        // {
        //     char printer_name[100];
        //     scanf("%s", printer_name);
        //     get_printer_supported_media(f, printer_name);
        // }
        // else if (strcmp(buf, "get-default-media") == 0)
        // {
        //     char printer_name[100];
        //     scanf("%s", printer_name);
        //     get_printer_default_media(f, printer_name);
        // }
        // else if (strcmp(buf, "get-default-orientation") == 0)
        // {
        //     char printer_name[100];
        //     scanf("%s", printer_name);
        //     get_printer_default_orientation(f, printer_name);
        // }
        // else if (strcmp(buf, "get-supported-color") == 0)
        // {
        //     char printer_name[100];
        //     scanf("%s", printer_name);
        //     get_printer_supported_color(f, printer_name);
        // }
        // else if (strcmp(buf, "get-supported-quality") == 0)
        // {
        //     char printer_name[100];
        //     scanf("%s", printer_name);
        //     get_printer_supported_quality(f, printer_name);
        // }
        // else if (strcmp(buf, "get-supported-orientation") == 0)
        // {
        //     char printer_name[100];
        //     scanf("%s", printer_name);
        //     get_printer_supported_orientation(f, printer_name);
        // }
        else if (strcmp(buf, "get-state") == 0)
        {
            char printer_name[100];
            char backend_name[100];
            scanf("%s%s", printer_name, backend_name);
            get_printer_state(f, printer_name, backend_name);
        }
        else if (strcmp(buf, "is-accepting-jobs") == 0)
        {
            char printer_name[100];
            char backend_name[100];
            scanf("%s%s", printer_name, backend_name);
            printer_is_accepting_jobs(f, printer_name, backend_name);
        }
        // else if (strcmp(buf, "get-default-resolution") == 0)
        // {
        //     char printer_name[100];
        //     scanf("%s", printer_name);
        //     get_printer_default_resolution(f, printer_name);
        // }
        // else if (strcmp(buf, "get-supported-resolution") == 0)
        // {
        //     char printer_name[100];
        //     scanf("%s", printer_name);
        //     get_printer_supported_resolution(f, printer_name);
        // }
        // else if (strcmp(buf, "get-default-color") == 0)
        // {
        //     char printer_name[100];
        //     scanf("%s", printer_name);
        //     get_printer_color_mode(f, printer_name);
        // }
        // else if (strcmp(buf, "help") == 0)
        // {
        //     display_help();
        // }
        // else if (strcmp(buf, "ping") == 0)
        // {
        //     char printer_name[100];
        //     scanf("%s", printer_name);
        //     pingtest(f, printer_name);
        // }
        // else if (strcmp(buf, "get-default-printer") == 0)
        // {
        //     char backend_name[100];
        //     scanf("%s", backend_name);
        //     get_default_printer(f, backend_name);
        // }
        // else if (strcmp(buf, "print-file") == 0)
        // {
        //     char printer_name[100], file_path[200];
        //     scanf("%s%s", printer_name, file_path);
        //     print_file(f, printer_name, file_path);
        // }
        else if (strcmp(buf, "get-active-jobs-count") == 0)
        {
            char printer_name[100];
            char backend_name[100];
            scanf("%s%s", printer_name, backend_name);
            get_active_jobs_count(f, printer_name, backend_name);
        }
        // else if (strcmp(buf, "get-all-jobs") == 0)
        // {
        //     int active_only;
        //     scanf("%d", &active_only);
        //     Job *j;
        //     int x = get_all_jobs(f, &j, active_only);
        //     int i;
        //     for(i=0;i<x;i++)
        //     {
        //         printf("%d .. %s  .. %s  .. %s  .. %s\n",j[i].job_id, j[i].title , j[i].printer , j[i].state , j[i].submitted_at);
        //     }
        // }
    }
}

void display_help()
{
    g_message("Available commands .. ");
    printf("%s\n", "stop");
    printf("%s\n", "refresh");
    printf("%s\n", "hide-remote-cups");
    printf("%s\n", "unhide-remote-cups");
    printf("%s\n", "hide-temporary-cups");
    printf("%s\n", "unhide-temporary-cups");
    printf("%s\n", "get-capabilities <printer name>");
    printf("get-all-options <printer-name>\n");
    printf("%s\n", "ping <printer name>");
    printf("%s\n", "get-default-printer <backend name>");
    // printf("%s\n","get-option-default <printer name> <option name>");
    // printf("%s\n","get-supported-raw <printer name> <option name>");
    printf("%s\n", "get-default-media <printer name>");
    printf("%s\n", "get-supported-media <printer name>");
    printf("%s\n", "get-default-resolution <printer name>");
    printf("%s\n", "get-supported-resolution <printer name>");
    printf("%s\n", "get-default-color <printer name>");
    printf("%s\n", "get-supported-color <printer name>");
    printf("print-file <printer-name> <file path>\n");
    printf("get-active-jobs-count <printer-name>\n");
    printf("get-all-jobs <0 for all jobs; 1 for only active>\n");

    printf("%s\n", "get-default-orientation <printer name>");
    // printf("%s\n","get-supported-quality <printer name>");
    printf("%s\n", "get-supported-orientation <printer name>");
    printf("%s\n", "get-state <printer name>");
    printf("%s\n", "is-accepting-jobs <printer name> <backend name(like \"CUPS\")>");
}
