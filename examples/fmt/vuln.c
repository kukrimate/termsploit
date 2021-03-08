// Compile with gcc -no-pie
#include <stdio.h>
#include <stdlib.h>

static void evil(const char *cmd)
{
    system(cmd);
}

int main()
{
	char buffer[255];

    for (;;) {
    	fgets(buffer, sizeof buffer, stdin);
        printf(buffer);
    }
}
