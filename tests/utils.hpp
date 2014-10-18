
#pragma once

template <typename Fn>
void repeat_n(size_t n, Fn && fn)
{
    for(auto i = 0; i < n; ++i)
        fn(i);
}
