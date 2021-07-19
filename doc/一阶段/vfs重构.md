# VFS重构

为了实现更加具有适用性的文件系统，决定增加VFS层来满足一切皆文件的抽象。

## IFS

文件接口

```C++
// fs implement
class IFS{
    virtual int init();
    virtual int open(const char *path, int flag);
    virtual int read(const char *path, char buf[], int count);
    // ...
}
```

## FS

文件接口实现

### FATFs

```c++
class FS : public IFS
```

### PipeFs

```c++
class PipeFs: public IFS
```

### StdoutFs

```c++
class StdoutFs : public IFS
```

## VFS

vfs是一颗抽象树，其上保存了所有可以使用的节点。

每个节点保持一个文件接口的指针，调用对应节点操作时通过该文件接口操作。

vfs同样会有和文件接口类似的函数，表示其虚拟实现，最终会调用文件接口。

```c++
// 抽象文件
class File
```

```c++
// 抽象树
class VFS{
    int init(IFS *ifs);
    static File *search(File *file, const char *path);
    int open(const char *path, int flag);
    int read(const char *path, char buf[], int count);
    // ...
}
```

## File_Describer

文件描述符抽象。该抽象主要服务于dup等系统调用，详见文件系统设计。