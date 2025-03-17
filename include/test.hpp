#pragma once

#include <string>
using std::string;

enum TestResultType {
    TEST_PASS,
    TEST_FAIL,
    TEST_ERROR
};

enum OutputFileType {
    OUTPUT_ALWAYS,
    OUTPUT_NEVER,
    OUTPUT_ON_SUCCESS,
    OUTPUT_ON_FAIL
};

typedef struct TestResult {
    string error;
    struct {
        TestResultType comparison = TEST_PASS;
        TestResultType performance = TEST_PASS;
    } result;
} TestResult;

typedef struct TestOptions {
    OutputFileType outputFile = OUTPUT_ON_FAIL;
    struct {
        bool task = false;
        bool result = true;
    } log;
    struct {
        bool enabled = true;
        bool ignoreWs = true;
    } comparison;
    struct {
        bool enabled = false;
        double maxTime = 1000;
    } performance;
    struct {
        string input = "input.hcss";
        string expected = "expected.css";
        string output = "output.css";
    } fileNames;
} TestOptions;

class Test {
    public:
        TestOptions options;
        string name;
        TestResult test();
        Test(string basePath, TestOptions options = {})
            : basePath(basePath),
            options(options),
            name("TEST \"" + basePath + '"')
        {};
        Test(string basePath, TestOptions options, string name)
            : basePath(basePath),
            options(options),
            name(name)
        {};
    private:
        string basePath;
};