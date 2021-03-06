#include "shell.h"
#include <stddef.h>
#include "clib.h"
#include <string.h>
#include "fio.h"
#include "filesystem.h"

#include "FreeRTOS.h"
#include "task.h"
#include "host.h"
#include <unistd.h>

typedef struct {
	const char *name;
	cmdfunc *fptr;
	const char *desc;
} cmdlist;

void ls_command(int, char **);
void man_command(int, char **);
void cat_command(int, char **);
void ps_command(int, char **);
void host_command(int, char **);
void help_command(int, char **);
void host_command(int, char **);
void mmtest_command(int, char **);
void test_command(int, char **);
void new_command(int, char **);
void _command(int, char **);

#define MKCL(n, d) {.name=#n, .fptr=n ## _command, .desc=d}

cmdlist cl[]={
	MKCL(ls, "List directory"),
	MKCL(man, "Show the manual of the command"),
	MKCL(cat, "Concatenate files and print on the stdout"),
	MKCL(ps, "Report a snapshot of the current processes"),
	MKCL(host, "Run command on host"),
	MKCL(mmtest, "heap memory allocation test"),
	MKCL(help, "help"),
	MKCL(test, "test new function"),
	MKCL(new, "create a new task"),
	MKCL(, "")
};

int parse_command(char *str, char *argv[]){
	int b_quote=0, b_dbquote=0;
	int i;
	int count=0, p=0;
	for(i=0; str[i]; ++i){
		if(str[i]=='\'')
			++b_quote;
		if(str[i]=='"')
			++b_dbquote;
		if(str[i]==' '&&b_quote%2==0&&b_dbquote%2==0){
			str[i]='\0';
			argv[count++]=&str[p];
			p=i+1;
		}
	}
	/* last one */
	argv[count++]=&str[p];

	return count;
}

void ls_command(int n, char *argv[]){
    fio_printf(1,"\r\n"); 
    int dir;
    if(n == 0){
        dir = fs_opendir("");
    }else if(n == 1){
        dir = fs_opendir(argv[1]);
        //if(dir == )
    }else{
        fio_printf(1, "Too many argument!\r\n");
        return;
    }
(void)dir;   // Use dir
}

int filedump(const char *filename){
	char buf[128];

	int fd=fs_open(filename, 0, O_RDONLY);

	if( fd == -2 || fd == -1)
		return fd;

	fio_printf(1, "\r\n");

	int count;
	while((count=fio_read(fd, buf, sizeof(buf)))>0){
		fio_write(1, buf, count);
    }
	
    fio_printf(1, "\r");

	fio_close(fd);
	return 1;
}

void ps_command(int n, char *argv[]){
	signed char buf[1024];
	vTaskList(buf);
        fio_printf(1, "\n\rName          State   Priority  Stack  Num\n\r");
        fio_printf(1, "*******************************************\n\r");
	fio_printf(1, "%s\r\n", buf + 2);	
}

void cat_command(int n, char *argv[]){
	if(n==1){
		fio_printf(2, "\r\nUsage: cat <filename>\r\n");
		return;
	}

    int dump_status = filedump(argv[1]);
	if(dump_status == -1){
		fio_printf(2, "\r\n%s : no such file or directory.\r\n", argv[1]);
    }else if(dump_status == -2){
		fio_printf(2, "\r\nFile system not registered.\r\n", argv[1]);
    }
}

void man_command(int n, char *argv[]){
	if(n==1){
		fio_printf(2, "\r\nUsage: man <command>\r\n");
		return;
	}

	char buf[128]="/romfs/manual/";
	strcat(buf, argv[1]);

    int dump_status = filedump(buf);
	if(dump_status < 0)
		fio_printf(2, "\r\nManual not available.\r\n");
}

void host_command(int n, char *argv[]){
    int i, len = 0, rnt;
    char command[128] = {0};

    if(n>1){
        for(i = 1; i < n; i++) {
            memcpy(&command[len], argv[i], strlen(argv[i]));
            len += (strlen(argv[i]) + 1);
            command[len - 1] = ' ';
        }
        command[len - 1] = '\0';
        rnt=host_action(SYS_SYSTEM, command);
        fio_printf(1, "\r\nfinish with exit code %d.\r\n", rnt);
    } 
    else {
        fio_printf(2, "\r\nUsage: host 'command'\r\n");
    }
}

void help_command(int n,char *argv[]){
	int i;
	fio_printf(1, "\r\n");
	for(i = 0;i < sizeof(cl)/sizeof(cl[0]) - 1; ++i){
		fio_printf(1, "%s - %s\r\n", cl[i].name, cl[i].desc);
	}
}

void test_command(int n, char *argv[]) {
	fio_printf(1, "\r\n");
	
	if(n != 2){
		fio_printf(1, "Usage: test [fib|prime]\r\n");
		return;
	}else{
		char tmpnum[128];
		int number = 0;
		if(strcmp(argv[1], "fib") == 0){

			int previous = -1, result = 1, i = 0, sum = 0;
			
			fio_printf(1, "Please insert a number for fibonacci: ");
			fio_read(0, tmpnum, 128);
			atoi(tmpnum, &number);
			
			for (i = 0; i <= number; i++){
				sum = result + previous;
				previous = result;
				result = sum;
			}	

    			fio_printf(1, "\r\nfibonacci of %d is %d\r\n", number, result);	
		}else if(strcmp(argv[1], "prime") == 0){
			
			fio_printf(1, "Please insert a number for prime number test: ");
			fio_read(0, tmpnum, 128);
			atoi(tmpnum, &number);	
			
			if(number <= 3)
				if(number > 1)
					fio_printf(1, "\r\n%d is prime\r\n", number);
				else	
					fio_printf(1, "\r\n%d is not prime\r\n", number);
			else if( number % 2 == 0 || number % 3 == 0)
				fio_printf(1, "\r\n%d is not prime\r\n", number);
			else{
				unsigned short i = 5;
				for(; i * i <= number; i += 6){
					if(number % i == 0 || number % (i+2) == 0){
						fio_printf(1, "\r\n%d is not prime\r\n", number); 
						return;
					}
				}
				fio_printf(1, "\r\n%d is prime\r\n", number); 
			}
		}else{
			fio_printf(1, "Usage: test [fib|prime]\r\n");
			return;
		}
	}
	
}

void blank_task(void *pvParameters){
	/* task body */
	/* DEBUG:	fio_printf(1, "Task is running...\r\n"); */
	while(1){}

}

void new_command(int n, char *argv[]){

	fio_printf(1, "\r\n");

	if(n != 2 || !is_int(argv[1]))
	{
		fio_printf(1, "usage: new [count]\r\n  - count: number of task to create\r\n");
		return;
	}

	int count = 0; 
	atoi(argv[1], &count);

	fio_printf(1, "%d\r\n", count);

	int i;
	for(i=0; i < count; i++){	
		portBASE_TYPE status = xTaskCreate(blank_task,			// task function pointer "pvTaskcode"
			 	   (signed portCHAR *) "New Task", // task name "pcName"
			 	   configMINIMAL_STACK_SIZE, 	// allocated stack size "usStackDepth"
			 	   NULL, 			// parameters passed to task "*pvParameters"
			 	   1, 				// (tmp) priority "uxPriority"
			 	   NULL); 			// handle
					  
		if(status == pdTRUE){
			fio_printf(1, "New task created.\r\n");
		} else if( status == errCOULD_NOT_ALLOCATE_REQUIRED_MEMORY){
			fio_printf(1,  "Failed to create new task: not enough memory to allocate.");
		}
}	
	fio_printf(1, "\r\n");
}

void _command(int n, char *argv[]){
    (void)n; (void)argv;
    fio_printf(1, "\r\n");
}

cmdfunc *do_command(const char *cmd){

	int i;

	for(i=0; i<sizeof(cl)/sizeof(cl[0]); ++i){
		if(strcmp(cl[i].name, cmd)==0)
			return cl[i].fptr;
	}
	return NULL;	
}
