#include "Util/Logger/logger.hpp"
#include "Util/Test/test.hpp"
#include <iostream>
#include <string>
#include <vector>

string testPath = "/workspace/HCSS/Test/";
std::vector<TestResult> results;

void test(string name, TestOptions options = {}) {
    results.push_back(Test(testPath + name, options, name).test());
}

int main(int argc, char **argv) {
    // Tests
    // test("Nesting");
    // test("Performance", {OUTPUT_NEVER, {false}, {false}, {true, 1500}});
    // test("Media Queries");
    test("Mixin", {OUTPUT_ALWAYS});
    // test("Events", {OUTPUT_ALWAYS, {false}, {false}});
    // test("Any", {OUTPUT_NEVER, {false}, {false}});
    
    // Check if all passed
    bool pass = true;
    for (auto result : results) {
        if (result.result.comparison != TEST_PASS || result.result.performance != TEST_PASS) {
            pass = false;
            break;
        }
    }
    if (pass) {
        Logger::log("All tests passed", LOG_SUCCESS);
    }
    else {
        Logger::log("One or more tests failed", LOG_FAIL);
    }
    return 0;
}