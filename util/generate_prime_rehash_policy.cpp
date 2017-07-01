#include <iostream>
#include <sstream>
#include <unordered_set>
#include <vector>
using namespace std;

int f(int n) {
    unordered_set<char> a;
    a.reserve(n);
    return a.bucket_count();
}

int main() {
    const int UPPER_LIMIT = 1e7;
    vector<int> buckets;

    int n = 1;
    int b = f(n);
    buckets.push_back(b);

    while (n < UPPER_LIMIT) {
        int step = 1;
        while (f(n + step) == b) {
            step *= 2;
        }
        step /= 2;
        while (step > 0) {
            if (f(n + step) == b) {
                n += step;
            }
            step /= 2;
        }
        ++n;
        b = f(n);
        buckets.push_back(b);
        cerr << n << endl;
    }
    string res = "{\n";
    ostringstream ss;
    for (int i = 0; i < (int)buckets.size(); ++i) {
        if (ss.str().size() >= 70) {
            res += ss.str() + "\n";
            ss.str("");
            ss.clear();
        }
        ss << buckets[i];
        if (i + 1 != (int)buckets.size()) {
            ss << ", ";
        }
    }
    res += ss.str() + "\n};\n";
    cout << res << endl;
}
