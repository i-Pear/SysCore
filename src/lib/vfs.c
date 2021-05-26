#include "vfs.h"
#include "stl.h"
#include "../driver/fatfs/ff.h"

Supernode vfs_super_node;

Inode* vfs_create_inode(enum Inode_Type inode_type, char* name){
    Inode* inode = (Inode*) k_malloc(sizeof(Inode));
    inode->inode_type = inode_type;
    inode->next = null;
    inode->first_child = null;
    inode->name = (char*) k_malloc(strlen(name) + 1);
    strcpy(inode->name, name);
    return inode;
}

FRESULT scan_files (Inode **inode,
        char* path        /* Start node to be scanned (***also used as work area***) */
)
{
    FRESULT res;
    DIR dir;
    UINT i;
    static FILINFO fno;


    res = f_opendir(&dir, path);                       /* Open the directory */
    if (res == FR_OK) {
        for (;;) {
            res = f_readdir(&dir, &fno);                   /* Read a directory item */
            if (res != FR_OK || fno.fname[0] == 0) break;  /* Break on error or end of dir */
            if (fno.fattrib & AM_DIR) {                    /* It is a directory */
                *inode = vfs_create_inode(INODE_TYPE_DIR, fno.fname);
                i = strlen(path);
                sprintf(&path[i], "/%s", fno.fname);
                res = scan_files(&(*inode)->first_child, path);                    /* Enter the directory */
                inode = &(*inode)->next;
                if (res != FR_OK) break;
                path[i] = 0;
            } else {                                       /* It is a file. */
                *inode = vfs_create_inode(INODE_TYPE_FILE, fno.fname);
                inode = &(*inode)->next;
            }
        }
        f_closedir(&dir);
    }

    return res;
}


void vfs_init(){
    vfs_super_node.root_inode = *vfs_create_inode(INODE_TYPE_DIR, "/");
    scan_files(&vfs_super_node.root_inode.first_child, "");
}

Inode* vfs_search(Inode *inode, char* path){
    if(strcmp(inode->name, "/") == 0){
        if(strcmp(path, "/") == 0)return inode;
        else inode = inode->first_child;
    }

    while (*path == '/')path++;
    char* end = path;
    while (*end != '/' && *end != '\0')end++;

    char* name = (char *)k_malloc(end - path + 1);
    int name_cur = 0;
    for(char* i = path; i != end; i++)name[name_cur++] = *i;
    name[name_cur] = '\0';
    Inode *p = inode;
    while(p && strcmp(p->name, name) != 0){
        p = p->next;
    }
    if(p && strcmp(p->name, name) == 0){
        if(*end == '\0'){
            return p;
        }else{
            return vfs_search(inode->first_child, end);
        }
    }
    return null;
}

Inode* vfs_open(char* path, int flag, int mode){
return null;
}