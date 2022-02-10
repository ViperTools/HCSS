#include "Util/Test/test.hpp"
#include <iostream>
#include <string>
#include <vector>

string testPath = "/workspace/HCSS/Test/";

void test(string name, TestOptions options = {}) {
    Test(testPath + name, options).test();
}

int main() {
    test("Nesting");
    test("Performance", {OUTPUT_NEVER, {true}, {false}, {true, 1500}});
    return 0;
}