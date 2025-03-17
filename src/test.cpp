#include <test.hpp>
#include <hcss/lexer/lexer.hpp>
#include <hcss/parser/types.hpp>
#include <hcss/parser/parser.hpp>
#include <transpiler.hpp>
#include <task.hpp>
#include <cctype>
#include <fstream>
#include <sstream>

TestResult Test::test() {
    TestResult res;
    Task task(name);

    try {
        bool pass = true;
        
        // Transpile to CSS
        std::ifstream file(basePath + '/' + options.fileNames.input);
        auto lexResult = task.call<vector<Token>>("LEXING", [this, &file] { return Lexer(file).lex(); });
        auto parseResult = task.call<vector<SyntaxNode>>("PARSING", [=] { return Parser(lexResult.result).parse(); });
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