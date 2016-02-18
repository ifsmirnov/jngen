#pragma once
#include <bits/stdc++.h>

namespace impl {

class Any {
public:
    std::string s;
    int i;

    Any(std::string s) : s(s), i(0) {}
    Any(const char *s) : s(s), i(0) {}
    Any(int i) : s(), i(i) {}

    Any(Any&& other) : s(other.s), i(other.i) {}
    Any(const Any& other) : s(other.s), i(other.i) {}

    operator std::string() const { return s; }
    operator int() const { return i; }

    template<typename T> T as() const {
        return (T)(*this);
    }
};

class Trait {
public:
    std::string name;
    Any val;
};

typedef std::map<std::string, Any> TraitMap;

struct TraitHelper {
    TraitHelper(std::string name) : name(name) {}
    std::string name;
    Trait operator=(Any x) { return {name, std::move(x)}; }
};

int collectTraits(TraitMap& map, const Trait& trait) {
    map.emplace(trait.name, std::move(trait.val));
    return 0;
}

std::pair<std::string, Any> collectTrait(const Trait& trait) {
    return { trait.name, trait.val };
}

__attribute__((noinline))
__attribute__((error("\n"
"  ***  Do not use $smth vars as f($smth, ...), only as f($smth = true/0.5/\"hello\", ...)")))
std::pair<std::string, Any> collectTrait(const TraitHelper&);

template<typename T>
__attribute__((noinline))
__attribute__((error("\n"
"  ***  Named arguments must follow positional ones")))
std::pair<std::string, Any> collectTrait(const T&);

template<typename... Traits>
TraitMap collectTraits(Traits... traits) {
    return { collectTrait(traits)... };
}

} // namespace impl

#define DECLARE_NAMED_PARAMETER(name)\
    impl::TraitHelper $ ## name(#name)
