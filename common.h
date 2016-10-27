#pragma once

#include <bits/stdc++.h>

void ensure(bool cond) {
    // TODO(ifsmirnov): make more readable asserts
    assert(cond);
}

void ensure(bool cond, const std::string& explanation) {
    assert(cond);
    (void)explanation;
}
