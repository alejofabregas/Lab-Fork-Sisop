#include <stdlib.h>
#include <stdio.h>

#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#define READ 0
#define WRITE 1

void filtro(int *fds_izq);
void enviar_filtrados(
        int *fds_izq, int pipe_izq, int *fds_der, int pipe_der, int primo);

int
main(int argc, char *argv[])
{
	int cant_nums = atoi(argv[1]);
	if (argc != 2 || cant_nums < 2) {
		perror("Error en los argumentos (solo se acepta un numero "
		       "entero mayor o igual a 2)");
		exit(-1);
	}

	int fds_der[2];
	int pipe_der = pipe(
	        fds_der);  // Creo el primer pipe para conectar el generador de numeros con los filtros
	if (pipe_der < 0) {
		perror("Error en pipe");
		exit(-1);
	}

	pid_t pid = fork();
	if (pid < 0) {
		printf("Error en fork: %d\n", pid);
		exit(-1);
	}

	if (pid == 0) {  // Proceso hijo
		close(fds_der[WRITE]);  // Cierro el file descriptor de escritura que no va a utilizar el filtro
		filtro(fds_der);  // Ejecuto el primer filtro
		close(fds_der[READ]);  // Cierro el file descriptor de lectura utilizado por el filtro

	} else {  // Proceso padre
		close(fds_der[READ]);  // Cierro el file descriptor de lectura que no va a utilizar el padre
		for (int num = 2; num <= cant_nums; num++) {
			pipe_der = write(fds_der[WRITE], &num, sizeof(num));  // Genero y envio los numeros al primer filtro por el pipe
			if (pipe_der < 0) {
				perror("Error en write");
				exit(-1);
			}
		}
		close(fds_der[WRITE]);  // Cierro el file descriptor de escritura que utilizo el padre
		wait(NULL);
	}
	exit(0);
}

void
filtro(int *fds_izq)
{
	close(fds_izq[WRITE]);  // Cierro el file descriptor de escritura que no se va a utilizar
	int primo = 0;
	int pipe_izq = read(fds_izq[READ], &primo, sizeof(primo));
	if (pipe_izq < 0) {
		perror("Error en read");
		exit(-1);
	}
	if (pipe_izq ==
	    0) {  // Si es cero, llega un EOF y no hay mas numeros para leer, termine
		close(fds_izq[READ]);
		exit(0);
	}
	printf("primo %d\n", primo);

	int fds_der[2];
	int pipe_der = pipe(
	        fds_der);  // Creo un nuevo pipe para conectar este filtro con el siguiente
	if (pipe_der < 0) {
		perror("Error en pipe");
		exit(-1);
	}

	pid_t pid = fork();
	if (pid < 0) {
		printf("Error en fork: %d\n", pid);
		exit(-1);
	}

	if (pid == 0) {  // Proceso hijo (siguiente filtro)
		close(fds_izq[READ]);  // Cierro el file descriptor de lectura que no va a utilizar el filtro
		close(fds_der[WRITE]);  // Cierro el file descriptor de escritura que no va a utilizar el filtro
		filtro(fds_der);
		close(fds_der[READ]);  // Cierro el file descriptor de lectura utilizado por el filtro

	} else {  // Proceso padre (filtro actual)}
		close(fds_der[READ]);  // Cierro el file descriptor de lectura que no va a utilizar el padre
		enviar_filtrados(fds_izq, pipe_izq, fds_der, pipe_der, primo);
		close(fds_izq[READ]);  // Cierro el file descriptor de lectura
		                       // utilizado por el padre para recibir los numeros del filtro anterior
		close(fds_der[WRITE]);  // Cierro el file descriptor de escritura
		                        // utilizado por el padre para enviar los numeros al siguiente filtro
		wait(NULL);
	}
}

void
enviar_filtrados(int *fds_izq, int pipe_izq, int *fds_der, int pipe_der, int primo)
{
	int n_recibido;
	while ((pipe_izq = read(fds_izq[READ], &n_recibido, sizeof(n_recibido)))) {  // Mientras no este bloqueado el pipe y me lleguen numeros
		if (pipe_izq < 0) {
			perror("Error en read");
			exit(-1);
		}
		if (n_recibido % primo != 0) {  // No son multiplos
			pipe_der = write(
			        fds_der[WRITE],
			        &n_recibido,
			        sizeof(n_recibido));  // Los mando al siguiente filtro
			if (pipe_der < 0) {
				perror("Error en write");
				exit(-1);
			}
		}
	}
}