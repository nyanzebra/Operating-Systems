#include <unistd.h>
#include <stdio.h>

int main() {
	int mypid = getpid();
	reboot(RB_REBOOT);
	return 0;
}