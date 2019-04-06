/*
  FUSE: Filesystem in Userspace
  Copyright (C) 2001-2007  Miklos Szeredi <miklos@szeredi.hu>

  Minor modifications and note by Andy Sayler (2012) <www.andysayler.com>

  Source: fuse-2.8.7.tar.gz examples directory
  http://sourceforge.net/projects/fuse/files/fuse-2.X/

  This program can be distributed under the terms of the GNU GPL.
  See the file COPYING.

  gcc -Wall `pkg-config fuse --cflags` fusexmp.c -o fusexmp `pkg-config fuse --libs`

  Note: This implementation is largely stateless and does not maintain
        open file handels between open and release calls (fi->fh).
        Instead, files are opened and closed as necessary inside read(), write(),
        etc calls. As such, the functions that rely on maintaining file handles are
        not implmented (fgetattr(), etc). Those seeking a more efficient and
        more complete implementation may wish to add fi->fh support to minimize
        open() and close() calls and support fh dependent functions.

*/

#define FUSE_USE_VERSION 28
#define HAVE_SETXATTR

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef linux
/* For pread()/pwrite() */
#define _XOPEN_SOURCE 500
#endif

#include <fuse.h>
#include <fuse_common.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <errno.h>
#include <sys/time.h> 
#include <fuse_lowlevel.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <dirent.h>
#include <fcntl.h>
#include <unistd.h>
#include <assert.h>


#ifdef HAVE_SETXATTR
#include <sys/xattr.h>
#endif

DIR *save_dir;
int tamanoBufferLog = 0;
char *archivo_log = "/home/miguel/Escritorio/log.csv";
char *archivo_historial = "/home/miguel/Escritorio/historial.csv";
char *pathRecycle = "/filesystem_201503666/recycle";
char *ultimoPathAccedido;
int NUM_REGISTROS_MAX = 1000;
/******************************************************************/
/*****************************************************************/
/*****************************************************************/

//static const char *practica_path = "/filesystem_201503666";
static int xmp_getattr(const char *path, struct stat *stbuf)
{
	int res;
	fprintf(stderr, "!getattr! '%s'\n", path);
	res = lstat(path, stbuf);
	if (res == -1){
		return -errno;
	}
	return 0;
}

static int xmp_access(const char *path, int mask)
{
	int res;
	res = access(path, mask);
	fprintf(stderr, "!access! %s %d\n", path, mask);
	if (res == -1){
		fprintf(stderr, "Error de acceso a %s\n", path);
		return -errno;
	}
	if(mask == 1){
		ultimoPathAccedido = (char *) calloc(strlen(path) + 1, sizeof(char));
		strcpy(ultimoPathAccedido, path);
	}
	return 0;
}

static int xmp_readlink(const char *path, char *buf, size_t size)
{
	int res;
	fprintf(stderr, "!readlink! %s\n", path);
	res = readlink(path, buf, size - 1);
	if (res == -1)
		return -errno;

	buf[res] = '\0';
	return 0;
}


static int xmp_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
		       off_t offset, struct fuse_file_info *fi)
{
	DIR *dp;
	struct dirent *de;

	(void) offset;
	(void) fi;
	fprintf(stderr, "!readdir! %s\n", path);
	dp = opendir(path);
	if (dp == NULL)
		return -errno;

	while ((de = readdir(dp)) != NULL) {
		struct stat st;
		memset(&st, 0, sizeof(st));
		st.st_ino = de->d_ino;
		st.st_mode = de->d_type << 12;
		if (filler(buf, de->d_name, &st, 0))
			break;
	}

	closedir(dp);
	return 0;
}

static int xmp_mknod(const char *path, mode_t mode, dev_t rdev)
{
	int res;
	fprintf(stderr, "!mknod! %s\n", path);
	/* On Linux this could just be 'mknod(path, mode, rdev)' but this
	   is more portable */
	if (S_ISREG(mode)) {
		res = open(path, O_CREAT | O_EXCL | O_WRONLY, mode);
		if (res >= 0)
			res = close(res);
	} else if (S_ISFIFO(mode))
		res = mkfifo(path, mode);
	else
		res = mknod(path, mode, rdev);
	if (res == -1)
		return -errno;

	return 0;
}

/* Function with behaviour like `mkdir -p'  */
static int mkpath(const char *s, mode_t mode){
        char *q, *r = NULL, *path = NULL, *up = NULL;
        int rv;
        rv = -1;
        if (strcmp(s, ".") == 0 || strcmp(s, "/") == 0)
                return (0);
        if ((path = strdup(s)) == NULL)
                exit(1);
        if ((q = strdup(s)) == NULL)
                exit(1);
        if ((r = dirname(q)) == NULL)
                goto out;
        if ((up = strdup(r)) == NULL)
                exit(1);
        if ((mkpath(up, mode) == -1) && (errno != EEXIST))
                goto out;
        if ((mkdir(path, mode) == -1) && (errno != EEXIST))
                rv = -1;
        else
                rv = 0;
out:
        if (up != NULL)
              free(up);
        free(q);
        free(path);
        return (rv);
}

static int xmp_mkdir(const char *path, mode_t mode)
{
	/*
	fprintf(stderr, "!mkdir! %s\n", path);
	char cadena[strlen(path)+1];
	strcpy(cadena, path);
	fprintf(stderr, ".............Creando directorios anidados............\n");
	char *ptrToken; // crea un apuntador char 
	char acumulado[strlen(path)+1];
	ptrToken = strtok( cadena, "/" ); 
	strcpy(acumulado, "");
	int res, resacc, resmkd;
	struct stat buf;
	struct fuse *f;

	while ( ptrToken != NULL ) { 
	   	fprintf(stderr, "Analizando => %s\n", ptrToken );
        strcat(acumulado, "/");
        strcat(acumulado, ptrToken);
	   	fprintf(stderr, "..Acumulado ---> %s\n", acumulado);
		memset(&buf, 0, sizeof(buf));		
		DIR* dir = opendir(acumulado);
		if(dir)
		{
			closedir(dir);
		}else if (ENOENT == errno)
		{
			resmkd = mkdir(acumulado, mode);
			if (resmkd == -1){
				fprintf(stderr, "Error creando directorio...%s\n", path);
			}else{
				fprintf(stderr, "Directorio creado...%s\n", path);
			}
		}else
		{
		

		}
		res = lstat(acumulado, &buf);
		if (res == -1){
			fprintf(stderr, "No existe el directorio...%s\n", path);
			resmkd = mkdir(acumulado, mode);
			if (resmkd == -1){
				fprintf(stderr, "Error creando directorio...%s\n", path);
			}else{
				fprintf(stderr, "Directorio creado...%s\n", path);
			}
			resacc = access(acumulado, fuse_get_context()->umask);
			if (resacc == -1){
				fprintf(stderr, "Error de acceso a %s\n", path);
			}
		}else{
			fprintf(stderr, "Existe...%s\n", acumulado);
			resacc = access(acumulado, fuse_get_context()->umask);
			if (resacc == -1){
				fprintf(stderr, "Error de acceso a %s\n", path);
			}
		}
		/ *res = access(acumulado, fuse_get_context()->umask);
		fprintf(stderr, "!access! %s\n", path);
		if (res == -1){
			fprintf(stderr, "Error de acceso a %s\n", path);
		}
		res = mkdir(acumulado, mode);
		if (res == -1){
			fprintf(stderr, "Error creando directorio...%s\n", path);
		}else{
			fprintf(stderr, "Directorio creado...%s\n", path);
		}
        ptrToken = strtok( NULL, "/" );
	} 
	*/

	int res;
	//res = mkdir(path, mode);
	char cadena[strlen(path)+9];
	strcpy(cadena, "mkdir -p ");
	strncat(cadena, path);
	res = system(cadena);
	fprintf(stderr, "!crear dir! %s\n", path);
	if (res == -1){
		fprintf(stderr, "Error creando directorio...%s\n", path);
		return -errno;
	}
	return 0;
}

static int xmp_mkdir_init(const char *path, mode_t mode)
{
	int res;
	fprintf(stderr, "!mkdir_init! %s\n", path);
	res = mkdir(path, mode);
	if (res == -1){
	fprintf(stderr, "Error creando directorio...%s\n", path);
		return -errno;
	}
	return 0;
}


static int xmp_unlink(const char *path)
{
	//int res;
	fprintf(stderr, "!unlink! %s\n", path);
	//res = unlink(path);
	//if (res == -1)
	//	return -errno;

	char cadena[strlen(path)+1];
	strcpy(cadena, path);
	char *ptrToken; // crea un apuntador char 
	char nombrearch[strlen(path)+1];
	ptrToken = strtok( cadena, "/" ); 
	//int res;
	while ( ptrToken != NULL ) { 
        strcpy(nombrearch, ptrToken);
        ptrToken = strtok( NULL, "/" );
	}  

	char cadena2[strlen(path)+1];
	strcpy(cadena2, path);
	char *ptrToken2; 
	char acumulado[strlen(path)+1];
	ptrToken2 = strtok( cadena2, "/" ); 
	strcpy(acumulado, "");

	while ( ptrToken2 != NULL ) { 
        if(strcmp(ptrToken2, nombrearch) != 0){
        	strcat(acumulado, "/");
        	strcat(acumulado, ptrToken2);
       		//fprintf(stderr, "..Acumulado ---> %s\n", acumulado);
        }
        ptrToken2 = strtok( NULL, "/" );
	} 


	/** CREANDO LAS CARPETAS EN RECYCLE **
	char *tempCarpetaRec = (char *)calloc(strlen(pathRecycle)+strlen(acumulado)+1, sizeof(char));
	strcpy(tempCarpetaRec, pathRecycle);
	strcat(tempCarpetaRec, acumulado);
	//mkpath(tempCarpetaRec, 0777);
	/************************************/
	fprintf(stderr, "\n\n");
	fprintf(stderr, "*********************************************\n");
	fprintf(stderr, "**   MOVIENDO A LA PAPELERA DE RECICLAJE   **\n");
	fprintf(stderr, "**   NOMBRE DEL ARCHIVO: %s\n", nombrearch);
	fprintf(stderr, "*********************************************\n");
	
	int tamPathRecycle = strlen(pathRecycle) + strlen(nombrearch) + 1;
	
	char pathRecycleArchivo[tamPathRecycle];
	strcpy(pathRecycleArchivo, pathRecycle);
	strcat(pathRecycleArchivo, "/");
	strcat(pathRecycleArchivo, nombrearch);
	fprintf(stderr, "\n'%s' -> '%s'\n\n", path, pathRecycleArchivo);

	rename(path, pathRecycleArchivo);
	int tamNuevaL = (strlen(nombrearch))+1+(strlen(acumulado))+1+(strlen(path))+1;
	char nuevalinea[tamNuevaL];
	strcpy(nuevalinea, nombrearch);
	strcat(nuevalinea, ",");
	strcat(nuevalinea, acumulado);
	strcat(nuevalinea, ",");
	strcat(nuevalinea, path);
	strcat(nuevalinea, "\n");
	nuevalinea[tamNuevaL] = '\0';
	int fd = open(archivo_log, O_WRONLY | O_APPEND);
	if (fd == -1){
			fprintf(stderr, "Error Abriendo Archivo LOG!\n");
			return -errno;
	}
	write(fd, nuevalinea, tamNuevaL);
	close(fd);
	tamanoBufferLog = tamanoBufferLog + tamNuevaL;

	int fdh = open(archivo_historial, O_WRONLY | O_APPEND);
	if (fdh == -1){
			fprintf(stderr, "Error Abriendo Archivo Historial!\n");
			return -errno;
	}
	char lineahis[tamNuevaL + sizeof(",ELIMINAR")];
	strcpy(lineahis, nombrearch);
	strcat(lineahis, ",");
	strcat(lineahis, acumulado);
	strcat(lineahis, ",");
	strcat(lineahis, path);
	strcat(lineahis, ",ELIMINAR");
	strcat(lineahis, "\n");
	lineahis[tamNuevaL + sizeof(",ELIMINAR")] = '\0';
	write(fdh, lineahis, sizeof(lineahis) - 1);
	close(fdh);
	return 0;
}

static int xmp_rmdir(const char *path)
{
	int res;
	fprintf(stderr, "!rmdir! %s\n", path);
	res = rmdir(path);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_symlink(const char *from, const char *to)
{
	int res;
	fprintf(stderr, "!symlink! %s\n", from);
	res = symlink(from, to);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_rename(const char *from, const char *to)
{
	int res;
	fprintf(stderr, "!rename! %s\n", from);
	res = rename(from, to);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_link(const char *from, const char *to)
{
	int res;
	fprintf(stderr, "!link! %s\n", from);
	res = link(from, to);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_chmod(const char *path, mode_t mode)
{
	int res;
	fprintf(stderr, "!chmod! %s\n", path);
	res = chmod(path, mode);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_chown(const char *path, uid_t uid, gid_t gid)
{
	int res;
	fprintf(stderr, "!chown! %s\n", path);
	res = lchown(path, uid, gid);
	if (res == -1)
		return -errno;

	return 0;
}

static int restaurarUnoAUno(char *linea, int tipo){

    char *nombrearch;
    char *pathPapeleraArchivo;
    char *pathAntiguo;

    char *ptrTokenInterno; 
    char *lecturaLinea = (char *)calloc(strlen(linea), sizeof(char));
    strcpy(lecturaLinea, linea);
    ptrTokenInterno = strtok(lecturaLinea, ",");
    int conteo = 1;
    int restaurar = 0;
    
    char *bufNuevoCSV = (char *)calloc(tamanoBufferLog, sizeof(char));
    strcpy(bufNuevoCSV, "");
    while ( ptrTokenInterno != NULL ) { 
        //fprintf(stderr, "@'%s' \n", ptrTokenInterno);
        	if(1 == conteo){
        		nombrearch = (char *)calloc(strlen(ptrTokenInterno)+1, sizeof(char));
        		strcpy(nombrearch, ptrTokenInterno);
				
				pathPapeleraArchivo = (char *)calloc(strlen(pathRecycle)+1+strlen(ptrTokenInterno)+1, sizeof(char));
        		strcpy(pathPapeleraArchivo, pathRecycle);
        		strcat(pathPapeleraArchivo, "/");
        		strcat(pathPapeleraArchivo, ptrTokenInterno);

        		//fprintf(stderr, "'%s' \n", pathPapeleraArchivo);
        		if(2 == tipo){
        			// TODO
        			restaurar = 1;
        		}else if(1 == tipo){
        			// Esperar a la siguiente comprobacion
        			restaurar = 0;
        		}
        		
        	}else if(2 == conteo){
       			if(strcmp(ptrTokenInterno, ultimoPathAccedido)==0){
        			restaurar = 1;
        		}
        	}else if(3 == conteo){
        		if(restaurar == 1){        				
        			pathAntiguo = (char *)calloc(strlen(ptrTokenInterno)+1, sizeof(char));
        			strcpy(pathAntiguo, ptrTokenInterno);
        			fprintf(stderr, "..............................................................\n");
					fprintf(stderr, ".. Restaurando .. '%s' \n", nombrearch);
					fprintf(stderr, "..   Moviendo .. '%s' -> '%s' \n", pathPapeleraArchivo, pathAntiguo);
					int resultado = rename(pathPapeleraArchivo, pathAntiguo);
					if(resultado == -1){
						fprintf(stderr, ".. ERROR %d! ......................................................\n\n", -errno);
					}else{
						fprintf(stderr, ".. OK ! ......................................................\n\n");	
						int fdh = open(archivo_historial, O_WRONLY | O_APPEND);
						if (fdh == -1){
							fprintf(stderr, "Error Abriendo Archivo Historial!\n");
							return -errno;
						}
						char lineahis[strlen(linea) + sizeof(",RESTAURAR")];
						strcpy(lineahis, linea);
						strcat(lineahis, ",RESTAURAR");
						strcat(lineahis, "\n");
						write(fdh, lineahis, sizeof(lineahis));
						close(fdh);
					}
					
        		}else{
        			// AGREGAR A NUEVO CSV
        			strcat(bufNuevoCSV, linea);
        			strcat(bufNuevoCSV, "\n");
        			int fd = open(archivo_log, O_WRONLY | O_APPEND);
					if (fd == -1){
							fprintf(stderr, "Error Abriendo Archivo!\n");
							return -errno;
					}
					write(fd, linea, strlen(linea));
					write(fd, "\n", 1);
					close(fd);
        		}
        		conteo = 1;
        		restaurar = 0;
        	}
        	conteo++;
        ptrTokenInterno = strtok( NULL, ",");
    }
    return 0;
}

static int xmp_truncate(const char *path, off_t size)
{
	/*int res;
	res = truncate(path, size);
	if (res == -1)
		return -errno;*/
	
	unlink(path);
	int tipo = 0;
	if(size == 2){
		// RESTAURAR TODO
		tipo = 2;
		fprintf(stderr, "\n\n\n");
		fprintf(stderr, "***************************************************************\n");
		fprintf(stderr, "**        RESTAURANDO TODO ...                               **\n");
		fprintf(stderr, "***************************************************************\n");
	}else{
		// RESTAURAR SOLO ESTA CARPETA
		tipo = 1;
		fprintf(stderr, "\n\n\n");
		fprintf(stderr, "***************************************************************\n");
		fprintf(stderr, "**   RESTAURANDO ESTA CARPETA:                               **\n");
		fprintf(stderr, "**   NOMBRE: %s\n", ultimoPathAccedido);
		fprintf(stderr, "***************************************************************\n");
	}
	
	int res;
	char *buf = (char *)calloc(tamanoBufferLog, sizeof(char));
	int fd;
	//off_t offset;
	fd = open(archivo_log, O_RDONLY);
	if (fd == -1){
		fprintf(stderr, "Error Abriendo Archivo!\n");
		return -errno;
	}
	res = read(fd, buf, (size_t)tamanoBufferLog);
	if (res == -1){
		res = -errno;
		fprintf(stderr, "Error Leyendo Archivo!\n");
	}
	if(buf == NULL)
		fprintf(stderr, "Buffer NULO!\n");

	close(fd);
	char *ptrToken; 

	fprintf(stderr, "Tamano Buff: -l: %d -e: %d \n\n", res, tamanoBufferLog);	
	char buff1[tamanoBufferLog + 1];

	strcpy(buff1, buf);
	buff1[tamanoBufferLog] = '\0';

		char *registros[NUM_REGISTROS_MAX];
		int conteoRegistros = 0;
		ptrToken = strtok(buff1, "\n"); 
		while ( ptrToken != NULL ) { 
			registros[conteoRegistros] = (char *)calloc(strlen(ptrToken), sizeof(char));
        	strcpy(registros[conteoRegistros], ptrToken);
        	conteoRegistros++;
        	ptrToken = strtok( NULL, "\n" );
		}  
	fd = open(archivo_log, O_TRUNC);
	if (fd == -1){
			fprintf(stderr, "Error Abriendo Archivo para Borrar!\n");
			return -errno;
	}
	close(fd);
	int i;
	for (i = 0; i < conteoRegistros; i++)
	{
		//fprintf(stderr, "?%s? \n", registros[i]);
		restaurarUnoAUno(registros[i], tipo);
	}
	fprintf(stderr, "\n***************************************************************\n");
	fprintf(stderr, "-----   RECUPERACION EXITOSA !                               ----\n");
	fprintf(stderr, "***************************************************************\n\n\n");
	return 0;
}


static int xmp_utimens(const char *path, const struct timespec ts[2])
{
	int res;
	struct timeval tv[2];
	fprintf(stderr, "!utimes! %s\n", path);
	tv[0].tv_sec = ts[0].tv_sec;
	tv[0].tv_usec = ts[0].tv_nsec / 1000;
	tv[1].tv_sec = ts[1].tv_sec;
	tv[1].tv_usec = ts[1].tv_nsec / 1000;

	res = utimes(path, tv);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_open(const char *path, struct fuse_file_info *fi)
{
	int res;
	fprintf(stderr, "!open! %s\n", path);
	res = open(path, fi->flags);
	if (res == -1)
		return -errno;

	close(res);
	return 0;
}

static int xmp_read(const char *path, char *buf, size_t size, off_t offset,
		    struct fuse_file_info *fi)
{
	int fd;
	int res;
	fprintf(stderr, "!read! %s\n", path);
	(void) fi;
	fd = open(path, O_RDONLY);
	if (fd == -1)
		return -errno;

	res = pread(fd, buf, size, offset);
	if (res == -1)
		res = -errno;

	close(fd);
	return res;
}

static int xmp_write(const char *path, const char *buf, size_t size,
		     off_t offset, struct fuse_file_info *fi)
{
	int fd;
	int res;
	fprintf(stderr, "!write! %s\n", path);
	(void) fi;
	fd = open(path, O_WRONLY);
	if (fd == -1)
		return -errno;

	res = pwrite(fd, buf, size, offset);
	if (res == -1)
		res = -errno;

	close(fd);
	return res;
}

static int xmp_statfs(const char *path, struct statvfs *stbuf)
{
	int res;
	fprintf(stderr, "!statfs! %s\n", path);
	res = statvfs(path, stbuf);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_create(const char* path, mode_t mode, struct fuse_file_info* fi) {

    (void) fi;
	fprintf(stderr, "!create! %s\n", path);
    int res;
    res = creat(path, mode);
    if(res == -1)
	return -errno;

    close(res);

    return 0;
}


static int xmp_release(const char *path, struct fuse_file_info *fi)
{
	/* Just a stub.	 This method is optional and can safely be left
	   unimplemented */
	fprintf(stderr, "!release! %s\n", path);
	(void) path;
	(void) fi;
	return 0;
}

static int xmp_fsync(const char *path, int isdatasync,
		     struct fuse_file_info *fi)
{
	/* Just a stub.	 This method is optional and can safely be left
	   unimplemented */
	fprintf(stderr, "!fsync! %s\n", path);
	(void) path;
	(void) isdatasync;
	(void) fi;
	return 0;
}

#ifdef HAVE_SETXATTR
static int xmp_setxattr(const char *path, const char *name, const char *value,
			size_t size, int flags)
{
	int res = lsetxattr(path, name, value, size, flags);
	fprintf(stderr, "!setxattr! %s\n", path);
	if (res == -1)
		return -errno;
	return 0;
}

static int xmp_getxattr(const char *path, const char *name, char *value,
			size_t size)
{
	int res = lgetxattr(path, name, value, size);
	fprintf(stderr, "!getxattr! %s\n", path);
	if (res == -1)
		return -errno;
	return res;
}

static int xmp_listxattr(const char *path, char *list, size_t size)
{
	int res = llistxattr(path, list, size);
	fprintf(stderr, "!listxattr! %s\n", path);
	if (res == -1)
		return -errno;
	return res;
}

static int xmp_removexattr(const char *path, const char *name)
{
	fprintf(stderr, "!removexattr! %s\n", path);
	int res = lremovexattr(path, name);
	if (res == -1)
		return -errno;
	return 0;
}


static void* xmp_init(struct fuse_conn_info *conn){
	//fchdir(save_dir);
	//close(save_dir);
	fprintf(stderr, "Inicializando...\n");
	xmp_mkdir_init("/filesystem_201503666/usr",0777);
	xmp_mkdir_init("/filesystem_201503666/usr/miguel_ruano",0777);
	xmp_mkdir_init("/filesystem_201503666/usr/miguel_ruano/desktop",0777);
	xmp_mkdir_init("/filesystem_201503666/tmp",0777);
	xmp_mkdir_init("/filesystem_201503666/etc",0777);
	xmp_mkdir_init("/filesystem_201503666/home",0777);
	xmp_mkdir_init("/filesystem_201503666/lib",0777);
	xmp_mkdir_init(pathRecycle,0777);

//	xmp_create("/filesystem_201503666/home/archivo",0777,stdout);
	//xmp_access("/filesystem_201503666/", 0);
	
	const char *data = "Miguel Angel Omar Ruano Roca \n 201503666 \n";
	int tamano = strlen(data);
	int fd = creat("/filesystem_201503666/home/archivo",0777);
	if(fd == -1){
		fprintf(stderr, "Error creando archivo. \n");
		return 0;
	}
	int	res = write(fd, data, tamano);
	if(res == -1){
		fprintf(stderr, "Error escribiendo archivo. \n");
		return 0;
	}
	int fd_log = creat(archivo_log, 0777);
	if(fd_log == -1){
		fprintf(stderr, "Error creando archivo de LOG \n");
		return 0;
	}
	close(fd_log);
	int fd_his = creat(archivo_historial, 0777);
	if(fd_his == -1){
		fprintf(stderr, "Error creando archivo de historial \n");
		return 0;
	}
	close(fd_his);
	return 0;
}

#endif /* HAVE_SETXATTR */

static struct fuse_operations xmp_oper = {
	.init		= xmp_init,
	.getattr	= xmp_getattr,
	.access		= xmp_access,
	.readlink	= xmp_readlink,
	.readdir	= xmp_readdir,
	.mknod		= xmp_mknod,
	.mkdir		= xmp_mkdir,
	.symlink	= xmp_symlink,
	.unlink		= xmp_unlink,
	.rmdir		= xmp_rmdir,
	.rename		= xmp_rename,
	.link		= xmp_link,
	.chmod		= xmp_chmod,
	.chown		= xmp_chown,
	.truncate	= xmp_truncate,
	.utimens	= xmp_utimens,
	.open		= xmp_open,
	.read		= xmp_read,
	.write		= xmp_write,
	.statfs		= xmp_statfs,
	.create     = xmp_create,
	.release	= xmp_release,
	.fsync		= xmp_fsync,
#ifdef HAVE_SETXATTR
	.setxattr	= xmp_setxattr,
	.getxattr	= xmp_getxattr,
	.listxattr	= xmp_listxattr,
	.removexattr	= xmp_removexattr,
#endif
};

int main(int argc, char *argv[])
{
	umask(0);
	//save_dir = open("/filesystem_201503666", 0);
	return fuse_main(argc, argv, &xmp_oper, NULL);
}
