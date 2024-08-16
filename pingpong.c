#include <stdlib.h>
#include <stdio.h>

#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#define READ 0
#define WRITE 1

int
main(void)
{
	int fds_der[2];
	int fds_izq[2];
	int pipe_der = pipe(fds_der);
	int pipe_izq = pipe(fds_izq);
	if (pipe_der < 0 || pipe_izq < 0) {
		perror("Error en pipe");
		exit(-1);
	}

	printf("Hola, soy PID <%d>:\n", getpid());
	printf("  - primer pipe me devuelve: [%d, %d]\n",
	       fds_der[READ],
	       fds_der[WRITE]);
	printf("  - segundo pipe me devuelve: [%d, %d]\n",
	       fds_izq[READ],
	       fds_izq[WRITE]);

	pid_t pid = fork();
	if (pid < 0) {
		printf("Error en fork: %d\n", pid);
		exit(-1);
	}

	if (pid == 0) {  // Proceso hijo, recibe un valor del padre y lo reenvia
		close(fds_der[WRITE]);  // Cierro los file descriptors que no voy a utilizar
		close(fds_izq[READ]);

		printf("\nDonde fork me devuelve %d:\n", pid);
		printf("  - getpid me devuelve: <%d>\n", getpid());
		printf("  - getppid me devuelve: <%d>\n", getppid());

		int recibido_hijo = 0;
		pipe_der = read(fds_der[READ],
		                &recibido_hijo,
		                sizeof(recibido_hijo));  // Recibo numero random
		if (pipe_der < 0) {
			perror("Error en read");
			exit(-1);
		}
		printf("  - recibo valor <%d> vía fd=%d\n",
		       recibido_hijo,
		       fds_der[READ]);
		close(fds_der[READ]);  // Cierro el file descriptor por donde
		                       // recibi el numero random enviado por el padre

		printf("  - reenvío valor en fd=%d y termino\n", fds_izq[WRITE]);
		pipe_izq = write(
		        fds_izq[WRITE],
		        &recibido_hijo,
		        sizeof(recibido_hijo));  // Reenvio el valor recibido al padre
		if (pipe_izq < 0) {
			perror("Error en write");
			exit(-1);
		}
		close(fds_izq[WRITE]);  // Cierro el file descriptor por donde reenvie el numero random al padre

	} else {  // Proceso padre, envia un valor random al hijo y lo vuelve a recibir
		close(fds_der[READ]);  // Cierro los file descriptors que no voy a utilizar
		close(fds_izq[WRITE]);

		srand(237);
		int num_random = rand();

		printf("\nDonde fork me devuelve <%d>:\n", pid);
		printf("  - getpid me devuelve: <%d>\n", getpid());
		printf("  - getppid me devuelve: <%d>\n", getppid());

		pipe_der =
		        write(fds_der[WRITE],
		              &num_random,
		              sizeof(num_random));  // Envio el valor random al hijo
		if (pipe_der < 0) {
			perror("Error en write");
			exit(-1);
		}
		printf("  - random me devuelve: <%d>\n", num_random);
		printf("  - envío valor <%d> a través de fd=%d\n",
		       num_random,
		       fds_der[WRITE]);
		close(fds_der[WRITE]);  // Cierro el file descriptor utilizado para enviar el numero random al hijo

		int recibido_padre = 0;
		pipe_izq = read(
		        fds_izq[READ], &recibido_padre, sizeof(recibido_padre));  // Recibo el numero random que le habia enviado al hijo
		if (pipe_izq < 0) {
			perror("Error en read");
			exit(-1);
		}

		// wait(NULL);

		printf("\nHola, de nuevo PID <%d>:\n",
		       getpid());  // Vuelvo al proceso original
		printf("  - recibí valor <%d> vía fd=%d\n",
		       recibido_padre,
		       fds_izq[READ]);

		close(fds_izq[READ]);  // Cierro el file descriptor utilizado para recibir el valor del hijo

		wait(NULL);  // Para matar al proceso zombie, no para sincronizar los pipes
		// Antes estaba sincronizando los pipes con el wait colocandolo
		// antes del print Hola, de nuevo PID <x> Para solucionarlo tuve
		// que cambiar el orden de los prints y los close de los file
		// descriptors
	}
	exit(0);
}