// Compile with gcc -fno-stack-protector -z execstack
#include <stdio.h>

int main()
{
	char buffer[20];

	printf("Leak: %p\n", buffer);
	fgets(buffer, 60, stdin);
}
