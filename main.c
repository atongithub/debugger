#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ptrace.h>
#include <sys/types.h>   
#include <sys/wait.h>   
#include <unistd.h>      
#include <stdarg.h>
#include <sys/user.h>

/**
SIGTRAP -  sent by ptrace for breakpoints
SIGKILL - force terminate
SIGSTOP - pause process
SIGCONT - continue process
SIGSEGV - segfault :P
SIGINT - ctrl+C
*/
struct breakpoint{
    unsigned long addr;
    unsigned long orig_data;
    int enabled;
};
void procmsg(const char* format, ...) {
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
}
void run_target(const char* program); 
void run_debugger(pid_t child_pid);
void enableBreakPoint(pid_t pid,struct breakpoint *bp){
    long data= ptrace(PTRACE_PEEKTEXT,pid,(void*)bp->addr,0);
    if(data==-1){
        perror("PTRACE_PEEKTEXT");
	exit(1);
	}
    bp->orig_data = data;

    unsigned long int3 = (data & ~0xff) | 0xcc;

    if (ptrace(PTRACE_POKETEXT, pid,
               (void*)bp->addr,
               (void*)int3) == -1) {
        perror("PTRACE_POKETEXT");
        exit(1);
    }

    bp->enabled = 1;
}

void disable_breakpoint(pid_t pid, struct breakpoint *bp) {
    if (ptrace(PTRACE_POKETEXT, pid,
               (void*)bp->addr,
               (void*)bp->orig_data) == -1) {
        perror("Restore breakpoint");
        exit(1);
    }

    bp->enabled = 0;
}


int main(int argc, char** argv){
    pid_t child_pid;
    if(argc<2){
        fprintf(stderr,"Expecting a program name as an argument\n");
        return -1;
        }
    child_pid = fork();
    if(child_pid==0){
        run_target(argv[1]);
    }else if(child_pid>0){
        run_debugger(child_pid);
    }else{
        perror("Fork");
        return -1;
    }
    return 0;

}
void run_target(const char* program){
    procmsg("Target program started. Running %s\n",program);
    //allowing the parent process to track the child process
    if(ptrace(PTRACE_TRACEME,0,0,0)<0){
        perror("ptrace");
        return;
    }

    //replacing the child process with the target process
    execl(program,program,(char*)NULL);
}
/*Single stepping line execution
void run_debugger(pid_t child_pid){
    int wait_status;
    unsigned icounter =0;
    procmsg("debugger started\n");

    //waiting for the child exec to send a SIGTRAP once the new process is executed
    wait(&wait_status); 
    //wifstopped == true if child is stopped
    while (WIFSTOPPED(wait_status)){
        icounter++;
        struct user_regs_struct regs;
        //storing cpu regs into structure
         ptrace(PTRACE_GETREGS,child_pid,0,&regs);
         unsigned instr= ptrace(PTRACE_PEEKTEXT, child_pid,regs.rip,0);

         procmsg("instruction counter = %u rip register = 0x%08x instruction = 0x%08x\n",icounter,regs.rip,instr);
        if(ptrace(PTRACE_SINGLESTEP,child_pid,0,0)<0){
            perror("ptrace");
            return;
        }//runs loop till WIFEXITED is true
        wait(&wait_status);
    }
    
    procmsg("Child finished executing %u instructions\n",icounter);
}*/

//Breakpoint execution
void run_debugger(pid_t child_pid) {
    int wait_status;
    struct user_regs_struct regs;

    procmsg("debugger has started\n");

    // wait for initial SIGTRAP after exec
    waitpid(child_pid, &wait_status, 0);

    if (!WIFSTOPPED(wait_status)) {
        fprintf(stderr, "Child did not stop as expected\n");
        return;
    }

    // asking user for breakpoint address from the objdump
    char input[64];
    unsigned long addr;

    printf("Enter breakpoint address in hex: ");
    fgets(input, sizeof(input), stdin);

    // convert string to unsigned long
    addr = strtoul(input, NULL, 16);

    if (addr == 0) {
        printf("Invalid address.\n");
        return;
    }

    struct breakpoint bp;
    bp.addr = addr;
    bp.enabled = 0;

    enableBreakPoint(child_pid, &bp);

    printf("Breakpoint set at 0x%lx\n", addr);

    // continue execution
    ptrace(PTRACE_CONT, child_pid, 0, 0);
    waitpid(child_pid, &wait_status, 0);

    if (WIFSTOPPED(wait_status) &&
        WSTOPSIG(wait_status) == SIGTRAP) {

        ptrace(PTRACE_GETREGS, child_pid, 0, &regs);

        // RIP is after INT3
        regs.rip -= 1;
        ptrace(PTRACE_SETREGS, child_pid, 0, &regs);

        disable_breakpoint(child_pid, &bp);

        printf("Breakpoint hit at 0x%llx\n", regs.rip);

        // execute original instruction
        ptrace(PTRACE_SINGLESTEP, child_pid, 0, 0);
        waitpid(child_pid, &wait_status, 0);

        // reinsert breakpoint
        enableBreakPoint(child_pid, &bp);

        printf("Continuing execution...\n");

        ptrace(PTRACE_CONT, child_pid, 0, 0);
        waitpid(child_pid, &wait_status, 0);
    }

    printf("Debugger exiting\n");
}

