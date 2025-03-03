#ifndef MANAGEMENT_CONSOLE_H
#define MANAGEMENT_CONSOLE_H

/* Starts the admin console thread that polls for commands from a file. */
void start_admin_console_thread(void);

/* Stops the admin console thread. */
void stop_admin_console_thread(void);

#endif // ADMIN_CONSOLE_H
