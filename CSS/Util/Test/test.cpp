#include "test.hpp"
#include "../../Lexer/Lexer.hpp"
#include "../../Parser/Types.hpp"
#include "../../Parser/BaseParser.hpp"
#include "../../Transpiler/Transpiler.hpp"
#include "../Logger/Task.hpp"
#include <cctype>
#include <fstream>
#include <sstream>

TestResult Test::test() {
    TestResult res;
    Task task(name);
    try {
        bool pass = true;
        
        // Transpile to CSS
        auto lexResult = task.call<vector<Token>>("LEXING", [this] { return Lexer(basePath + '/' + options.fileNames.input).lex(); });
        auto parseResult = task.call<vector<SyntaxNode>>("PARSING", [=] { return BaseParser(lexResult.result).parse(); });
        auto css = task.call<wstring>("TRANSPILING", [=] { Transpiler t; t.visit(parseResult.result); return t.source; }).result;        

        // Performance Test
        if (options.performance.enabled) {
            if (task.stopwatch.elapsed > options.performance.maxTime) {
                pass = false;
                res.result.performance = TEST_FAIL;
            }
        }

        // Comparison Test
        if (options.comparison.enabled) {
            std::wstringstream buffer;
            buffer << std::wifstream(basePath + '/' + options.fileNames.expected, std::ios::in).rdbuf();
            std::wstring expected = buffer.str();
            if (options.comparison.ignoreWs) {
                std::erase_if(css, isspace);
                std::erase_if(expected, isspace);
            }
            if (css != expected) {
                pass = false;
                res.result.comparison = TEST_FAIL;
            }
        }

        // Logs
        if (options.log.task) {
            task.log();
        }
        
        if (options.log.result) {
            Logger::log(task.name, pass ? LOG_SUCCESS : LOG_FAIL);
        }

        // Output to File
        if (options.outputFile == OUTPUT_ALWAYS || options.outputFile == (pass ? OUTPUT_ON_SUCCESS : OUTPUT_ON_FAIL)) {
            std::wofstream(basePath + '/' + options.fileNames.output, std::ios::out) << css;
        }
    }
    catch(const std::exception& e) {
        if (options.log.result) {
            Logger::log(task.name + ": " + e.what(), LOG_ERROR);
        }
        res.error = e.what();
    }
    return res;
}