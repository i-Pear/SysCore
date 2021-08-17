#pragma once

#include "../lib/stl/stl.h"

void init_vdso();

extern size_t vdso_text_page;
extern size_t vdso_data_page;