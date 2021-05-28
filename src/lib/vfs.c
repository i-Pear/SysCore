#include "vfs.h"
#include "stl.h"
#include "../driver/fatfs/ff.h"

Supernode vfs_super_node;

Inode* vfs_create_inode(char* name, int flag, InodeData* data, Inode* father, Inode* previous){
    Inode* inode = (Inode*) k_malloc(sizeof(Inode));
    inode->data = data;
    inode->flag = flag;
    inode->next = null;
    inode->first_child = null;
    inode->father = father;
    inode->previous = previous;
    inode->name = (char*) k_malloc(strlen(name) + 1);
    strcpy(inode->name, name);
    return inode;
}

FRESULT scan_files (Inode **inode,Inode *father,
        char* path        /* Start node to be scanned (***also used as work area***) */
)
{
    FRESULT res;
    DIR dir;
    UINT i;
    static FILINFO fno;


    res = f_opendir(&dir, path);                       /* Open the directory */
    if (res == FR_OK) {
        Inode * previous = null;
        for (;;) {
            res = f_readdir(&dir, &fno);                   /* Read a directory item */
            if (res != FR_OK || fno.fname[0] == 0) break;  /* Break on error or end of dir */
            if (fno.fattrib & AM_DIR) {                    /* It is a directory */
                *inode = vfs_create_inode(fno.fname, S_IFDIR, null, father, previous);
                i = strlen(path);
                sprintf(&path[i], "/%s", fno.fname);
                res = scan_files(&(*inode)->first_child,*inode, path);                    /* Enter the directory */
                previous = *inode;
                inode = &(*inode)->next;
                if (res != FR_OK) break;
                path[i] = 0;
            } else {                                       /* It is a file. */
                *inode = vfs_create_inode(fno.fname, S_IFREG, null, father, previous);
                previous = *inode;
                inode = &(*inode)->next;
            }
        }
        f_closedir(&dir);
    }

    return res;
}

// 传进来的路径一定以/开头
Inode* _vfs_search(Inode* inode, char* path){
    // 去除先导/
    while (*path == '/')path++;
    char* p = path;
    // 找到下一个/或者结尾
    while (*p != '/' && *p != '\0')p++;
    // 缓存*p，因为接下来要修改*p
    char tmp = *p;
    *p = '\0';

//    printf("path %s\n", path);

    // 当前文件夹
    if(strcmp(path, ".") == 0){
        *p = tmp;
        // 进入下一级
        // 找到本级文件夹里的第一个文件
        while (inode->previous)inode = inode->previous;
        return _vfs_search(inode, p);
    }
    // 上级文件夹
    if(strcmp(path, "..") == 0){
        *p = tmp;
        // 找到上级文件夹里的第一个文件
        Inode * father = inode->father;
        while(father->previous){
            father = father->previous;
        }
        return _vfs_search(father, p);
    }

    // 遍历该文件夹下的文件
    Inode* dir_file = inode;
//    printf("path: %s\n", path);
    while (dir_file){
//        printf("dir file %s\n", dir_file->name);
        if(strcmp(path, dir_file->name) == 0){
            // 恢复*p;
            *p = tmp;
            // 查找结束
            if(*p == '\0'){
                return dir_file;
            }
            // 进入下一级
            return _vfs_search(dir_file->first_child, p);
        }
        dir_file = dir_file->next;
    }
    *p = tmp;
    // 没找到
    return null;
}

// 查找目录时不能以/结尾
Inode* vfs_search(char* path, char* cwd){
//    printf("vfs search %s, cwd %s\n", path, cwd);

    // 路径不能是空
    if(strlen(path) <= 0)return null;
    // 绝对路径
    if(path[0] == '/'){
        if(strcmp(path, "/") == 0){
            // 根目录，直接返回
            return &vfs_super_node.root_inode;
        }
        return _vfs_search(vfs_super_node.root_inode.first_child, path);
    }
    // 相对路径
    // 相对路径必须以/结尾
    if(cwd != null){
        assert(cwd[strlen(cwd) - 1] == '/')
    }
    char* new_path = (char*) k_malloc(strlen(path) + strlen(cwd) + 2);
    sprintf(new_path, "%s%s", cwd, path);
    if(strcmp(new_path, "/") == 0){
        // 根目录，直接返回
        k_free((size_t)new_path);
        return &vfs_super_node.root_inode;
    }
    Inode *res = _vfs_search(vfs_super_node.root_inode.first_child, new_path);
    k_free((size_t)new_path);
    return res;
}

Inode *vfs_create_file(char* path, int flag){
//    printf("vfs create path %s\n", path);

    // 无法创建根目录
    assert(strlen(path) > 0 && strcmp(path, "/") != 0)
    // 查找最后一个/
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

    Inode *parent = vfs_search(father, null);

    // 必须要有父节点
    assert(parent != null);

//    printf("vfs creat parent: %s\n", parent->name);

    Inode* previous = null;
    if(parent->first_child != null){
        previous = parent->first_child;
        // 找到该父目录的最后一个文件
        while (previous->next)previous = previous->next;
    }
    Inode *ret = null;
    if(flag & S_IFDIR){
        f_mkdir(path + 1);
        ret = vfs_create_inode(path + 1, flag, null, parent, previous);
    }else if(flag & S_IFREG){
        FIL fil;
        FRESULT result = f_open(&fil, path, FA_CREATE_NEW | FA_WRITE | FA_READ);
        if(result != FR_OK){
            printf("cannot create file: %s\n", path);
            panic("")
        }
        f_close(&fil);
        ret = vfs_create_inode(path + 1, flag, null, parent, previous);
    }else if(flag & S_IFCHR){
        ret = vfs_create_inode("console", S_IFCHR, null, parent, previous);
    }else{
        printf("unknown flag: 0x%x\n", flag);
        panic("")
    }
    // previous如果是空就代表着父目录是一个空文件夹
    if(previous == null){
        parent->first_child = ret;
    }else{
        previous->next = ret;
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

    // TODO: 先简单处理一下
    if(strcmp("/.", father) == 0){
        father[1] = '\0';
    }
//    printf("father %s\n", father);

    Inode *parent = vfs_search(father, null);
    if(path == null){
        panic("parent cannot be null")
    }

    Inode* p = parent->first_child;
    if(p == null){
        return 0;
    }
//    printf("delete path: %s\n", path + i + 1);
    while(p){
        if(strcmp(p->name, path + i + 1) == 0){
            // TODO: 内存泄漏
            Inode * nn = parent->next->next;
            parent->next = nn;
            if(nn){
                nn->previous = parent;
            }
            return 0;
        }
        parent = p;
        p = p->next;
    }
    return 0;
}

Inode* vfs_open(char* path, char* cwd, int o_flag, int s_flag){
//    printf("vfs open find %s, cwd %s\n", path, cwd);

    Inode * find = vfs_search(path, cwd);

//    printf("vfs_open find %s, flag 0o%o\n", find->name, find->flag);
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
// TODO： 先简单的把./和.忽略过去
    if(path[0] == '.' && path[1] == '/')path += 2;
    if(s_flag & S_IFREG){
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

void print_tree(Inode* p){
    while (p){
        printf("i am %s;", p->name);
        if(p->father)printf("my father %s;", p->father->name);
        if(p->previous)printf("my previous %s;", p->previous->name);
        printf("my flag 0o%o",p->flag);
        printf("\n");
        if(p->first_child)print_tree(p->first_child);
        p = p->next;
    }
}

void vfs_init(){
    // 初始化根目录
    vfs_super_node.root_inode = *vfs_create_inode("/", S_IFDIR, null, null, null);

    // 初始化vfs树状结构
    scan_files(&vfs_super_node.root_inode.first_child, &vfs_super_node.root_inode,"");

//    print_tree(vfs_super_node.root_inode.first_child);

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