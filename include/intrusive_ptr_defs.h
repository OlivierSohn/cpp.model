#pragma once

namespace imajuscule
{
    enum {intrusive_ptr_zero_count = -1};
    // 0 means owned by one only (in order to optimize equality test)
}
