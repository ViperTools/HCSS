#include "Util/Test/test.hpp"
#include <iostream>
#include <string>
#include <vector>

string testPath = "/workspace/HCSS/Test/";

void test(string name, TestOptions options = {}) {
    Test(testPath + name, options).test();
}

int main(int argc, char **argv) {
    // test("Nesting");
    // test("Performance", {OUTPUT_ALWAYS, {true}, {false}, {true, 1500}});
    // test("Media Queries");
    test("Mixin");
    // test("Any", {OUTPUT_NEVER, {false}, {false}});
    return 0;
}