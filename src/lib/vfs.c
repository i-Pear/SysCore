#include "vfs.h"
#include "stl.h"
#include "../driver/fatfs/ff.h"

Supernode vfs_super_node;

Inode* vfs_create_inode(char* name, int flag, InodeData* data){
    Inode* inode = (Inode*) k_malloc(sizeof(Inode));
    inode->data = data;
    inode->flag = flag;
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
                *inode = vfs_create_inode(fno.fname, S_IFDIR, null);
                i = strlen(path);
                sprintf(&path[i], "/%s", fno.fname);
                res = scan_files(&(*inode)->first_child, path);                    /* Enter the directory */
                inode = &(*inode)->next;
                if (res != FR_OK) break;
                path[i] = 0;
            } else {                                       /* It is a file. */
                *inode = vfs_create_inode(fno.fname, S_IFREG, null);
                inode = &(*inode)->next;
            }
        }
        f_closedir(&dir);
    }

    return res;
}

Inode* vfs_search(Inode *inode, char* path){
    if(strcmp(inode->name, "/") == 0){
        if(strcmp(path, "/") == 0){
            return inode;
        }
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
    while(p && (strcmp(p->name, name) != 0)){
//        printf("vfs p.name %s\n", p->name);
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

Inode *vfs_create_file(char* path, int flag){
    int len = strlen(path);

    int i = len - 1;
    while(i >= 0 && path[i] != '/')i--;
    char* father = (char*) k_malloc(i + 2);
    memcpy(father, path, i + 1);
    // 根目录是/， 其他目录是/dev/，所以其他目录需要更短一些
    father[i + 1] = '\0';
    if(strlen(father) > 1){
        father[i] = '\0';
    }

//    printf("father: %s\n", father);

    Inode *parent = vfs_search(&vfs_super_node.root_inode, father);

//    printf("parent: 0x%x\n", parent);


    Inode *ret = null;
    if(flag & S_IFDIR){
//        printf("path %s\n", path + 1);
        f_mkdir(path + 1);
        ret = vfs_create_inode(path + 1, flag, null);
    }else if(flag & S_IFREG){
        FIL fil;
        FRESULT result = f_open(&fil, path, FA_CREATE_NEW | FA_WRITE | FA_READ);
        if(result != FR_OK){
            printf("cannot create file: %s\n", path);
            panic("")
        }
        f_close(&fil);
        ret = vfs_create_inode(path + 1, flag, null);
    }else if(flag & S_IFCHR){
        ret = vfs_create_inode("console", S_IFCHR, null);
    }else{
        printf("unknown flag: 0x%x\n", flag);
        panic("")
    }
    if(parent->first_child == null){
        parent->first_child = ret;
    }else{
        Inode *p = parent->first_child;
        if(p->next == null){
            p->next = ret;
        }else{
            while (p->next)p = p->next;
            p->next = ret;
        }
    }
    k_free((size_t)father);
    return ret;
}

int vfs_delete_inode(char* path){
    int len = strlen(path);

    int i = len - 1;
    while(i >= 0 && path[i] != '/')i--;
    char* father = (char*) k_malloc(i + 2);
    memcpy(father, path, i + 1);
    // 根目录是/， 其他目录是/dev/，所以其他目录需要更短一些
    father[i + 1] = '\0';
    if(strlen(father) > 1){
        father[i] = '\0';
    }

    // TODO: just simply delte /.
    father = "/";

    Inode *parent = vfs_search(&vfs_super_node.root_inode, father);

    Inode* p = parent->first_child;
    if(p == null){
        return 0;
    }
//    printf("delete path: %s\n", path + i + 1);
    while(p){
        if(strcmp(p->name, path + i + 1) == 0){
//            printf("!!!!!!!!!!!!\n");
            // TODO: 内存泄漏
            parent->next = parent->next->next;
            return 0;
        }
        parent = p;
        p = p->next;
    }
    return 0;
}

Inode* vfs_open(char* path, int o_flag, int s_flag){
    Inode * find = vfs_search(&vfs_super_node.root_inode, path);
//    printf("=> 0x%x o_flag 0x%x\n", find, o_flag);

//    printf("vfs find = 0x%x\n", find);
    // 检查是否有新建标记
    if(!find){
        if(!(o_flag & O_CREATE)){
            return null;
        }
        find = vfs_create_file(path, s_flag);
    }

//    printf("vfs find = 0x%x\n", find);
    // 检查是否是字符设备
    if(s_flag & S_IFCHR){
        return find;
    }
    BYTE mode = 0;
    if (o_flag & O_RDONLY)
        mode = FA_READ;
    if (o_flag & O_WRONLY)
         mode = FA_WRITE;
    if (o_flag & O_RDWR)
        mode = FA_READ | FA_WRITE;
    if(o_flag & O_DIRECTORY){
        if(!(s_flag & S_IFDIR)){
            panic("O_DIRECTORY must be used with S_IFDIR")
        }
    }
    if(o_flag == 0){
        mode = FA_READ | FA_WRITE;
    }

//    printf("vfs mode = 0x%x\n", mode);

    if(s_flag & S_IFREG){
//        printf("vfs path = 0x%x\n", path);

        InodeData *inodeData = (InodeData*) k_malloc(sizeof(InodeData));
        FRESULT res = f_open(&inodeData->fat32, path, mode);
        if(res != FR_OK)return null;
        find->data = inodeData;
    }else if(s_flag & S_IFDIR){
        InodeData *inodeData = (InodeData*) k_malloc(sizeof(InodeData));
        FRESULT res = f_opendir(&inodeData->fat32_dir, path);
        if(res != FR_OK)return null;
        find->data = inodeData;
    }else{
        printf("error s_flag: 0x%x\n", s_flag);
        panic("")
    }
    return find;
}

int vfs_read(Inode *inode,  char buf[], int count){
    if(inode->flag & S_IFREG){
        UINT read_bytes = 0;
        f_read(&inode->data->fat32, buf, count, &read_bytes);
        return (int)read_bytes;
    }
    printf("unsupported file flag: 0x%x\n", inode->flag);
    panic("")
    return -1;
}

int vfs_write(Inode* inode, char buf[], int count){
//    printf("inode 0x%x\n", inode);
//    printf("flag %x\n", inode->flag);
    if(inode->flag & S_IFCHR){
        for (int i = 0; i < count; ++i) {
            putchar(buf[i]);
        }
        return count;
    }
    else if(inode->flag & S_IFREG){
        UINT write_bytes = 0;
        f_write(&inode->data->fat32, buf, count, &write_bytes);
        return (int)write_bytes;
    }
    printf("unsupported file flag: 0x%x\n", inode->flag);
    panic("")
    return -1;
}

void vfs_close(Inode* inode){
    if(inode->flag & S_IFREG){
        f_close(&inode->data->fat32);
        k_free((size_t)inode->data);
        inode->data = null;
    }else if(inode->flag & S_IFDIR){
        f_closedir(&inode->data->fat32_dir);
        k_free((size_t)inode->data);
        inode->data = null;
    }else{
        panic("vfs_close only close dir & reg")
    }
}

Inode *vfs_mkdir(char* path, int flag){
//    printf("mkdir path: %s\n", path);
    flag |= S_IFDIR;
    return vfs_create_file(path, flag);
}

void vfs_init(){
    // 初始化根目录
    vfs_super_node.root_inode = *vfs_create_inode("/", S_IFDIR, null);

    // 初始化vfs树状结构
    scan_files(&vfs_super_node.root_inode.first_child, "");



    // 初始化stdout， 将其作为/dev/console文件
    // TODO: 现在将所有的S_IFCHR都视做stdout，这不合理
    FILINFO filinfo;
    FRESULT fr = f_stat("dev", &filinfo);
    if(fr != FR_OK){
        Inode * res = vfs_create_file("/dev", S_IFDIR);
        if(res == null){
            panic("can't create /dev");
        }
    }
    Inode * res = vfs_create_file("/dev/console", S_IFCHR);
    if(res == null){
        panic("can't create /dev/console");
    }
}

void test_isDirectory();
int isDirectoryInitialized = 0;
int vfs_isDirectory(char* absolutePath){
    if(isDirectoryInitialized == 0){
        isDirectoryInitialized = 1;
        test_isDirectory();
    }
    // fatfs can't distinguish /
    if(strcmp(absolutePath, "/") == 0){
        return 1;
    }

    FRESULT fr;
    FILINFO filinfo;
    fr = f_stat(absolutePath, &filinfo);

    if(fr != FR_OK){
        return 0;
    }

    if(filinfo.fattrib & AM_DIR){
        return 1;
    }
    return 0;
}

void test_isDirectory(){
    assert(vfs_isDirectory("/") == 1);
    assert(vfs_isDirectory("/mnt") == 1);
    assert(vfs_isDirectory("mnt") == 1);
    assert(vfs_isDirectory("uname") == 0);
}