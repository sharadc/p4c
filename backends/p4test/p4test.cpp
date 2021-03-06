/*
Copyright 2013-present Barefoot Networks, Inc.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

#include "ir/ir.h"
#include "lib/log.h"
#include "lib/error.h"
#include "lib/exceptions.h"
#include "lib/gc.h"
#include "lib/crash.h"
#include "lib/nullstream.h"
#include "frontends/common/parseInput.h"
#include "frontends/p4/evaluator/evaluator.h"
#include "frontends/p4/frontend.h"
#include "frontends/p4/toP4/toP4.h"
#include "midend.h"

int main(int argc, char *const argv[]) {
    setup_gc_logging();
    setup_signals();

    CompilerOptions options;
    options.langVersion = CompilerOptions::FrontendVersion::P4_16;

    if (options.process(argc, argv) != nullptr)
        options.setInputFile();
    if (::errorCount() > 0)
        return 1;

    auto program = parseP4File(options);
    auto hook = options.getDebugHook();

    if (program != nullptr && ::errorCount() == 0) {
        P4::FrontEnd fe;
        fe.addDebugHook(hook);
        program = fe.run(options, program);
        if (program != nullptr && ::errorCount() == 0) {
            P4Test::MidEnd midEnd(options);
            midEnd.addDebugHook(hook);
#if 0
            /* doing this breaks the output until we get dump/undump of srcInfo */
            if (options.debugJson) {
                std::stringstream tmp;
                JSONGenerator gen(tmp);
                gen << program;
                JSONLoader loader(tmp);
                loader >> program;
            }
#endif
            (void)midEnd.process(program);
            if (options.dumpJsonFile)
                JSONGenerator(*openFile(options.dumpJsonFile, true)) << program << std::endl;
            if (options.debugJson) {
                std::stringstream ss1, ss2;
                JSONGenerator gen1(ss1), gen2(ss2);
                gen1 << program;

                const IR::Node* node = nullptr;
                JSONLoader loader(ss1);
                loader >> node;

                gen2 << node;
                if (ss1.str() != ss2.str()) {
                    error("json mismatch");
                    std::ofstream t1("t1.json"), t2("t2.json");
                    t1 << ss1.str() << std::flush;
                    t2 << ss2.str() << std::flush;
                    system("json_diff t1.json t2.json");
                }
            }
        }
    }
    if (Log::verbose())
        std::cerr << "Done." << std::endl;
    return ::errorCount() > 0;
}
