#ifndef OS_RISC_V_MEMORY_KEEPER_H
#define OS_RISC_V_MEMORY_KEEPER_H

#include "../../lib/stl/vector.h"
#include "../../lib/stl/RefCountPtr.h"

class _MemKeeper{
public:

    Vector<void*> elements;

    void add(void* p);

    void erase(void* p);

    ~_MemKeeper();
};

typedef RefCountPtr<_MemKeeper> MemKeeper;

#endif //OS_RISC_V_MEMORY_KEEPER_H
