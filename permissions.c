#include <stdio.h>
#include <string.h>


int is_allowed(const char *role, const char *command) {


if (!role || !command) {
return 0;
}


if (strcmp(role, "admin") == 0) {
return 1;
}


if (strcmp(role, "read") == 0) {

if (strncmp(command, "/list", 5) == 0) return 1;
if (strncmp(command, "/read", 5) == 0) return 1;
if (strncmp(command, "/search", 7) == 0) return 1;
if (strncmp(command, "/info", 5) == 0) return 1;

return 0;
}


return 0;
}
