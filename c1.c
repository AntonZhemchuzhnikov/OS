#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>

int main() {
	char* name = "parent";
	
	pid_t chpid = fork();
	pid_t pid = getpid();
	pid_t ppid = getppid();
	
	if (!chpid)
		name = "child";
	printf("%s: \n\t"
		"id = %d \n\t"
		"parent id = %d \n\t"
		"child id = %d \n\n", name, pid, ppid, chpid);
	return 0;
}
