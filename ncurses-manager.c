#include <ctype.h>
#include <dirent.h>
#include <locale.h>
#include <ncurses.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
/* The ncurses manager for desktop-gremlin-linux
 * This aims to allow control of your gremlin through a
 * keypad-like interface, for more fun and also a cool
 * vibe i guess (not sure what im saying)
 */

// implementation from: https://cocomelonc.github.io/linux/2024/09/16/linux-hacking-2.html
int
find_process_by_name (const char *proc_name)
{
    DIR *dir;
    struct dirent *entry;
    int pid = -1;

    dir = opendir ("/proc");
    if (dir == NULL)
    {
        perror ("opendir /proc failed");
        return -1;
    }

    while ((entry = readdir (dir)) != NULL)
    {
        if (isdigit (*entry->d_name))
        {
            char path[512];
            snprintf (path, sizeof (path), "/proc/%s/comm", entry->d_name);

            FILE *fp = fopen (path, "r");
            if (fp)
            {
                char comm[512];
                if (fgets (comm, sizeof (comm), fp) != NULL)
                {
                    // remove trailing newline from comm
                    comm[strcspn (comm, "\r\n")] = 0;
                    if (strcmp (comm, proc_name) == 0)
                    {
                        pid = atoi (entry->d_name);
                        fclose (fp);
                        break;
                    }
                }
                fclose (fp);
            }
        }
    }

    closedir (dir);
    return pid;
}

void
drawui (void)
{
    clear ();
    attron (COLOR_PAIR (1));
    mvprintw (0, 0, "desktop-gremlin-linux management tool v2.2.1 - press q to quit, use mouse");
    attroff (COLOR_PAIR (1));
    attron (COLOR_PAIR (2));
    mvprintw (1, 0, "╒═══════════════Event Log══════════════════╕");
    mvprintw (2, 0, "│                                          │");
    mvprintw (3, 0, "└──────────────────────────────────────────┘");
    attroff (COLOR_PAIR (2));
    mvprintw (5, 0, "CONTROLS:");
    attron (COLOR_PAIR (3));
    mvprintw (6, 2, " Kill gremlin ");
    mvprintw (8, 2, " FORCE kill ");
    mvprintw (10, 4, "W");
    mvprintw (11, 2, "A");
    mvprintw (11, 4, "S");
    mvprintw (11, 6, "D");
    mvprintw (13, 2, "Spawn!");
    attroff (COLOR_PAIR (4));
}

void
handle_sigwinch (int sig)
{
    (void)sig; // unused
    drawui ();
}

int
main (int argc, char *argv[])
{
    struct sigaction sa;
    sa.sa_handler = handle_sigwinch;
    sigemptyset (&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    sigaction (SIGWINCH, &sa, NULL);
    char *pname;
    int gpid;
    if (argc == 2)
        pname = argv[1];
    else if (argc > 2)
    {
        printf ("ERROR: expected less than 2 arguments, but got %d instead. Ignoring.\n  SYNTAX: [program name] [desktop-gremlin-linux process name]\n  OR:\n  [program name]\n", argc);
        exit (-1);
    }
    else
        pname = "Manhattan_Cafe";
    // Initialise curses!
    setlocale (LC_ALL, "");
    initscr ();
    start_color ();
    cbreak ();
    noecho ();
    // make getchr() non-blocking:
    //   - IMPT as we need to handle keypresses that repeat!
    nodelay (stdscr, TRUE);
    keypad (stdscr, TRUE);
    use_default_colors ();
    init_pair (1, COLOR_CYAN, -1);
    init_pair (2, COLOR_WHITE, COLOR_BLUE);
    init_pair (3, COLOR_RED, COLOR_WHITE);
    init_pair (4, COLOR_GREEN, COLOR_WHITE);
    // Draw the UI i guess
    drawui ();
    // start capturing mouse events
    mousemask (ALL_MOUSE_EVENTS | REPORT_MOUSE_POSITION, NULL);
    // event loop
    MEVENT event;
    int ch;
    while ((ch = getch ()) != 'q')
    {
        if (ch == KEY_MOUSE)
        {
            if (getmouse (&event) == OK)
            {
                attron (COLOR_PAIR (2));
                mvprintw (2, 0, "│                                          │");
                mvprintw (2, 1, "Mouse clicked at %d,%d", event.x, event.y);
                attroff (COLOR_PAIR (2));
                if (event.y == 6 && event.x < 16 && event.x > 1)
                {
                    // kill gremlin
                    attron (COLOR_PAIR (2));
                    mvprintw (2, 0, "│                                          │");
                    mvprintw (2, 1, "Killing gremlin...");
                    attroff (COLOR_PAIR (2));
                    gpid = find_process_by_name (pname);
                    attron (COLOR_PAIR (2));
                    mvprintw (2, 0, "│                                          │");
                    mvprintw (2, 1, "PID: %d", gpid);
                    attroff (COLOR_PAIR (2));
                    if (gpid > 0)
                        kill (gpid, SIGINT);
                    else
                    {
                        attron (COLOR_PAIR (2));
                        mvprintw (2, 0, "│                                          │");
                        mvprintw (2, 1, "Process not found: %d", gpid);
                        attroff (COLOR_PAIR (2));
                    }
                }
                else if (event.y == 8 && event.x < 14 && event.x > 1)
                {
                    // force kill
                    attron (COLOR_PAIR (2));
                    mvprintw (2, 0, "│                                          │");
                    mvprintw (2, 1, "force-killing gremlin...");
                    attroff (COLOR_PAIR (2));
                    gpid = find_process_by_name (pname);
                    attron (COLOR_PAIR (2));
                    mvprintw (2, 0, "│                                          │");
                    mvprintw (2, 1, "PID: %d", gpid);
                    attroff (COLOR_PAIR (2));
                    if (gpid > 0)
                        kill (gpid, SIGKILL);
                    else
                    {
                        attron (COLOR_PAIR (2));
                        mvprintw (2, 0, "│                                          │");
                        mvprintw (2, 1, "Process not found: %d", gpid);
                        attroff (COLOR_PAIR (2));
                    }
                }
                else if (event.y == 13 && event.x < 8 && event.x > 1)
                {
                    // force kill
                    attron (COLOR_PAIR (2));
                    mvprintw (2, 0, "│                                          │");
                    mvprintw (2, 1, "Spawning gremlin...");
                    pid_t pid = fork ();
                    if (pid == 0)
                    {
                        const char *home = getenv ("HOME");
                        if (!home)
                        {
                            fprintf (stderr, "HOME not set\n");
                            exit (1);
                        }
                        char path[512];
                        snprintf (path, sizeof (path), "%s/.local/bin/Manhattan_Cafe", home);
                        execlp (path, "Manhattan_Cafe", NULL);
                        perror ("exec failed");
                        exit (1);
                        // parent continues, child runs independently
                    }

                    attroff (COLOR_PAIR (2));
                    if (gpid > 0)
                        kill (gpid, SIGKILL);
                    else
                    {
                        attron (COLOR_PAIR (2));
                        mvprintw (2, 0, "│                                          │");
                        mvprintw (2, 1, "Process not found: %d", gpid);
                        attroff (COLOR_PAIR (2));
                    }
                }
            }
        }
        else if (ch == 'w')
        {
            attron (COLOR_PAIR (2));
            mvprintw (2, 0, "│                                          │");
            mvprintw (2, 1, "moving gremlin...");
            attroff (COLOR_PAIR (2));
            gpid = find_process_by_name (pname);
            attron (COLOR_PAIR (2));
            mvprintw (2, 0, "│                                          │");
            mvprintw (2, 1, "PID: %d", gpid);
            attroff (COLOR_PAIR (2));
            if (gpid > 0)
                kill (gpid, SIGRTMIN);
            else
            {
                attron (COLOR_PAIR (2));
                mvprintw (2, 0, "│                                          │");
                mvprintw (2, 1, "Process not found: %d", gpid);
                attroff (COLOR_PAIR (2));
            }
        }
        else if (ch == 'a')
        {
            attron (COLOR_PAIR (2));
            mvprintw (2, 0, "│                                          │");
            mvprintw (2, 1, "moving gremlin...");
            attroff (COLOR_PAIR (2));
            gpid = find_process_by_name (pname);
            attron (COLOR_PAIR (2));
            mvprintw (2, 0, "│                                          │");
            mvprintw (2, 1, "PID: %d", gpid);
            attroff (COLOR_PAIR (2));
            if (gpid > 0)
                kill (gpid, SIGRTMIN + 1);
            else
            {
                attron (COLOR_PAIR (2));
                mvprintw (2, 0, "│                                          │");
                mvprintw (2, 1, "Process not found: %d", gpid);
                attroff (COLOR_PAIR (2));
            }
        }
        else if (ch == 's')
        {
            attron (COLOR_PAIR (2));
            mvprintw (2, 0, "│                                          │");
            mvprintw (2, 1, "moving gremlin...");
            attroff (COLOR_PAIR (2));
            gpid = find_process_by_name (pname);
            attron (COLOR_PAIR (2));
            mvprintw (2, 0, "│                                          │");
            mvprintw (2, 1, "PID: %d", gpid);
            attroff (COLOR_PAIR (2));
            if (gpid > 0)
                kill (gpid, SIGRTMIN + 2);
            else
            {
                attron (COLOR_PAIR (2));
                mvprintw (2, 0, "│                                          │");
                mvprintw (2, 1, "Process not found: %d", gpid);
                attroff (COLOR_PAIR (2));
            }
        }
        else if (ch == 'd')
        {
            attron (COLOR_PAIR (2));
            mvprintw (2, 0, "│                                          │");
            mvprintw (2, 1, "moving gremlin...");
            attroff (COLOR_PAIR (2));
            gpid = find_process_by_name (pname);
            attron (COLOR_PAIR (2));
            mvprintw (2, 0, "│                                          │");
            mvprintw (2, 1, "PID: %d", gpid);
            attroff (COLOR_PAIR (2));
            if (gpid > 0)
                kill (gpid, SIGRTMIN + 3);
            else
            {
                attron (COLOR_PAIR (2));
                mvprintw (2, 0, "│                                          │");
                mvprintw (2, 1, "Process not found: %d", gpid);
                attroff (COLOR_PAIR (2));
            }
        }
    }
    // we have broken out.
    //  - now, exit cleanly
    endwin ();
}