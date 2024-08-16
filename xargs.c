#ifndef NARGS
#define NARGS 4
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#define POS_COMANDO 1

void ejecutar(char *comando, char **argumentos);
void resetear_args(char **argumentos, int cant_args);

int
main(int argc, char *argv[])
{
	if (argc != 2) {
		perror("Error en los argumentos (solo se acepta un comando)");
		exit(-1);
	}
	char *comando = argv[POS_COMANDO];
	char *argumentos[NARGS + 2] = {
		comando
	};  // Array de largo NARGS mas el primer argumento y el NULL del final
	char *linea = NULL;
	size_t largo_linea;
	int num_iter = 1;  // Agrego argumentos al vector desde la posicion 1
	while (getline(&linea, &largo_linea, stdin) != -1) {
		linea[strlen(linea) - 1] = '\0';  // Remuevo newline
		argumentos[num_iter] = strdup(linea);
		num_iter++;
		if (num_iter == NARGS + 1) {
			argumentos[num_iter] =
			        NULL;  // Agrego un NULL al final de los argumentos para que execvp deje de leer
			ejecutar(comando, argumentos);
			resetear_args(argumentos, NARGS);
			num_iter = 1;
		}
	}
	argumentos[num_iter] = NULL;
	ejecutar(comando, argumentos);
	resetear_args(argumentos, num_iter);
	free(linea);
	exit(0);
}

void
ejecutar(char *comando, char **argumentos)
{
	pid_t pid = fork();

	if (pid < 0) {
		printf("Error en fork: %d\n", pid);
		exit(-1);
	}

	if (pid == 0) {  // Proceso hijo ejecuta el comando
		execvp(comando, argumentos);
	} else {  // Proceso padre espera a que el hijo ejecute
		wait(NULL);
	}
}

void
resetear_args(char **argumentos, int cant_args)
{
	for (int i = 1; i <= cant_args; i++) {
		free(argumentos[i]);
	}
}