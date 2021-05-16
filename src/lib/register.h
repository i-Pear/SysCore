#ifndef __REGISTER_H__
#define __REGISTER_H__

#include "stddef.h"

// supervisor-level timer interrupts
#define REGISTER_SIP_STIE (1 << 5)
// the privilege level at which a hart was executing before entering supervisor mode
#define REGISTER_SSTATUS_SPP (1 << 8)
// whether supervisor interrupts were enabled prior to trapping into supervisor mode
#define REGISTER_SSTATUS_SPIE (1 << 5)

size_t register_read_sip();
size_t register_read_sstatus();
size_t register_read_satp();
size_t register_read_sscratch();

#endif

