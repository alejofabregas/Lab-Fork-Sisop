#define _GNU_SOURCE

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <unistd.h>
#include <sys/types.h>
#include <dirent.h>
#include <fcntl.h>

void buscar_item(char *nombre_item,
                 DIR *directorio,
                 char *ruta_parcial,
                 char *(*func_str)(const char *, const char *) );
void acceder_subdir(struct dirent *item_actual,
                    char *nombre_item,
                    DIR *directorio,
                    char *ruta_parcial,
                    char *(*func_str)(const char *, const char *) );
void imprimir_item(struct dirent *item_actual,
                   char *nombre_item,
                   char *ruta_parcial,
                   char *(*func_str)(const char *, const char *) );

int
main(int argc, char *argv[])
{
	DIR *directorio = opendir(".");  // Abro el directorio base
	if (directorio == NULL) {
		perror("Error de opendir");
		exit(-1);
	}

	char ruta_parcial[PATH_MAX] = "";

	if (argc == 2) {  // Case sensitive
		buscar_item(argv[1], directorio, ruta_parcial, strstr);
	} else if (argc == 3 && strcmp(argv[1], "-i") == 0) {  // Case insensitive
		buscar_item(argv[2], directorio, ruta_parcial, strcasestr);
	} else {
		perror("Error en los argumentos");
		exit(-1);
	}
	closedir(directorio);  // Cierro el directorio base
	exit(0);
}

// Busca recursivamente en todos los subdirectorios
void
buscar_item(char *nombre_item,
            DIR *directorio,
            char *ruta_parcial,
            char *(*func_str)(const char *, const char *) )
{
	struct dirent *item_actual;
	while ((item_actual = readdir(directorio))) {
		if (item_actual->d_type == DT_DIR) {  // El item es un subdirectorio
			if (!func_str(item_actual->d_name, ".") &&
			    !func_str(item_actual->d_name, "..")) {
				acceder_subdir(item_actual,
				               nombre_item,
				               directorio,
				               ruta_parcial,
				               func_str);
			}
		} else if (item_actual->d_type ==
		           DT_REG) {  // El item es un archivo
			imprimir_item(
			        item_actual, nombre_item, ruta_parcial, func_str);
		} else {  // El item es un archivo de tipo desconocido
			printf("%s%s es de tipo desconocido\n",
			       ruta_parcial,
			       item_actual->d_name);
		}
	}
}

void
acceder_subdir(struct dirent *item_actual,
               char *nombre_item,
               DIR *directorio,
               char *ruta_parcial,
               char *(*func_str)(const char *, const char *) )
{
	imprimir_item(item_actual, nombre_item, ruta_parcial, func_str);

	int fd_directorio = dirfd(directorio);
	int archivo_directorio =
	        openat(fd_directorio, item_actual->d_name, O_DIRECTORY);
	DIR *nuevo_directorio =
	        fdopendir(archivo_directorio);  // Genero el subdirectorio

	if (nuevo_directorio) {
		char nueva_ruta_parcial[PATH_MAX];  // Concateno la ruta del subdirectorio con la ruta anterior y el /
		strcpy(nueva_ruta_parcial, ruta_parcial);
		strcat(nueva_ruta_parcial, item_actual->d_name);
		strcat(nueva_ruta_parcial, "/");

		buscar_item(nombre_item,
		            nuevo_directorio,
		            nueva_ruta_parcial,
		            func_str);  // Llamado recursivo para buscar el item en el subdirectorio

		close(fd_directorio);
		close(archivo_directorio);
		closedir(nuevo_directorio);  // Cierro los FD del subdirectorio abierto
	}
}

void
imprimir_item(struct dirent *item_actual,
              char *nombre_item,
              char *ruta_parcial,
              char *(*func_str)(const char *, const char *) )
{
	if (func_str(item_actual->d_name, nombre_item)) {
		printf("%s%s\n", ruta_parcial, item_actual->d_name);
	}
}