// Compile with gcc -Wl,-z,relro,-z,now -fno-stack-protector
// Mitigations: relro, PIE, W^X
#include <stdio.h>

int main()
{
	char buffer[20];
    printf("main: %p\n", main);
	fgets(buffer, 80, stdin);
}
