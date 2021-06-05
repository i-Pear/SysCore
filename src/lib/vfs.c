#include "vfs.h"

int (*vfs_init)() = NULL;
int (*vfs_open)(const char* path, int flag) = NULL;
int (*vfs_read)(const char* path,  char buf[], int count) = NULL;
int (*vfs_write)(const char* path, char buf[], int count) = NULL;
int (*vfs_close)(const char* path) = NULL;
int (*vfs_mkdir)(const char* path, int flag) = NULL;
int (*vfs_link)(const char* path, int flag) = NULL;
int (*vfs_unlink)(const char* path) = NULL;
int (*vfs_mount)(const char* dist, char* origin) = NULL;
int (*vfs_umount)(const char* dist) = NULL;
int (*vfs_fstat)(const char* path, size_t* result, int option) = NULL;

void vfs_register() {

}
