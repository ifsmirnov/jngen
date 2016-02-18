#pragma once
#include <bits/stdc++.h>

void ensure(bool cond) {
    assert(cond);
}

void ensure(bool cond, const std::string& explanation) {
    assert(cond);
    (void)explanation;
}
