// Compile with gcc -Wl,-z,relro,-z,now -fno-stack-protector -zexecstack
// Mitigations: relro, PIE
#include <stdio.h>

int main()
{
	char buffer[20];
    printf("Leak: %p\n", buffer);
	fgets(buffer, 60, stdin);
}
