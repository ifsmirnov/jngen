#pragma once
#include <bits/stdc++.h>

#include "common.h"

namespace impl {

class Random {
private:
    RandomEngine engine_;
    bool hasEngine_;

    RandomEngine& engine() {
        return hasEngine_ ? engine_ : randomEngine;
    }

public:
    Random() : hasEngine_(false)
    {  }

    /* implicit */ Random(RandomEngine engine) :
            engine_(std::move(engine)),
            hasEngine_(true)
    {  }

    // Think about signed/unsigned types

    int next(int n) {
        return engine().next(static_cast<uint32_t>(n));
    }

    long long next(long long n) {
        return engine().next(static_cast<uint64_t>(n));
    }

    int next(int l, int r) {
        return l + next(r-l+1);
    }

    int next(size_t l, size_t r) {
        return l + next(static_cast<int>(r-l+1));
    }

    long long next(long long l, long long r) {
        return r + next(r-l+1);
    }

    template<typename T>
    std::vector<T> combination(T n, size_t k) {
        ensure(k <= n);
        std::unordered_map<T, T> used;
        std::vector<int> res;
        for (size_t i = 0; i < k; ++i) {
            T oldValue = used.count(n-i-1) ? used[n-i-1] : n-i-1;
            // TODO
            T index = next((T)(n-i));
            res.push_back(used.count(index) ? used[index] : index);
            used[index] = oldValue;
        }
        return res;
    }
};

} // namespace impl
