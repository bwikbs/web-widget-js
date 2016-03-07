#!/usr/bin/env python3

import os
import optparse
import sys
import itertools
import subprocess
import importlib

BASE_DIR = os.path.dirname(os.path.abspath(__file__))

TIMEOUT_DEFAULT = 60

TEST_SUITES = ["stress", "mozilla"]
SUPPORTED_ARCHS = ["x64", "x86", "arm64", "arm"]
VARIANTS = ["interpreter", "jit"]
MODES = ["debug", "release"]

class CommandOptionValues(object):
    def __init__(self, arch, variants, mode, timeout, suite):
        mode = mode.split(",")
        arch = arch.split(",")
        variants = variants.split(",")
        suite = suite.split(",")
        for m in mode:
            if not (m in MODES):
                raise ValueError("Mode should be between [" +', '.join(MODES) +"] not " + m)
        for v in variants:
            if not (v in VARIANTS):
                raise ValueError("Variants should be between [" +', '.join(VARIANTS) + "] not " + v )
        for a in arch:
            if not (a in SUPPORTED_ARCHS):
                raise ValueError("Architecture should be between [" + ', '.join(SUPPORTED_ARCHS) +"] not " + a)
        for s in suite:
            if not (s in TEST_SUITES):
                raise ValueError("TestSuite should be between [" + ', '.join(TEST_SUITES) +"] not " + s)
        if (timeout > 300):
            raise ValueError("Maximum wait time should be less than 300")

        self.suite = suite
        self.timeout = timeout
        self.arch_and_variants_and_mode = itertools.product(arch, variants, mode)

class ArgumentParser(object):
    def __init__(self):
        self._parser = self._create_option_parser()

    def _create_option_parser(self):
        parser = optparse.OptionParser()
        parser.add_option("-a", "--arch",
                help=("The architecture to run tests for %s" % SUPPORTED_ARCHS),
                default="x64")
        parser.add_option("--variants",
                help=("The variants to run tests for %s" % VARIANTS),
                default="interpreter")
        parser.add_option("-m", "--mode",
                help=("The modes to run tests for %s" % MODES),
                default="release")
        parser.add_option("-t", "--timeout",
                help=("The time out value for each test case, which should be between" % MODES),
                type=int,
                default=TIMEOUT_DEFAULT)
        parser.add_option("-s", "--suite",
                help=("The test suite to run test for %s" % TEST_SUITES),
                default="stress")
        return parser

    def parse(self, args):
        (options, paths) = self._parser.parse_args(args=args)
        suite = options.suite
        timeout = options.timeout
        mode = options.mode
        variants = options.variants
        arch = options.arch
        options = CommandOptionValues(arch, variants, mode, timeout, suite)
        return (paths, options)


class StressReader(object):
    def list_tests(self):
        test_base_dir = os.path.join("test", "JavaScriptCore", "stress")
        tc_stress = os.path.join(test_base_dir, "TC.stress")
        tc_list = []
        with open(tc_stress, "r") as f:
            for line in f:
                if not line.startswith("//"):
                    tc_list.append(os.path.join(test_base_dir, line[:len(line) - 1]))
        return tc_list

    def mandatory_file(self, fname):
        return ["test/JavaScriptCore/stress/test.js"]

    def output_file(self, a_v_m):
        return "test/JavaScriptCore/stress/jsc." + '.'.join(a_v_m) + ".gen.txt"

class MozillaReader(object):
    def list_tests(self):
        test_base_dir = os.path.join("test", "SpiderMonkey")
        test_dirs = ["ecma_5", "Intl", "js1_1", "js1_2", "js1_3", "js1_4", "js1_5",
                    "js1_6," ,"js1_7","js1_8", "js1_8_1", "js1_8_5"]
        tc_list = []
        for test_dir in test_dirs:
            for (path, dir, files) in sorted(os.walk(os.path.join(test_base_dir, test_dir))):
                for filename in files:
                    ext = os.path.splitext(filename)[-1]
                    if ext == '.js':
                        if filename.find("shell") == -1:
                            filepath = os.path.join(path, filename)
                            with open(filepath, "r") as f:
                                if not f.readline().startswith("// escargot-skip"):
                                    tc_list.append(filepath)
        return tc_list

    def mandatory_file(self, fname):
        if fname.find("ecma_5") != -1:
            if fname.find("JSON") != -1:
                return ["test/SpiderMonkey/shell.js", "test/SpiderMonkey/ecma_5/shell.js", "test/SpiderMonkey/ecma_5/JSON/shell.js"]
            elif fname.find("RegExp") != -1:
                return ["test/SpiderMonkey/shell.js", "test/SpiderMonkey/ecma_5/shell.js", "test/SpiderMonkey/ecma_5/RegExp/shell.js"]
            else:
                return ["test/SpiderMonkey/shell.js", "test/SpiderMonkey/ecma_5/shell.js"]
        elif fname.find("js1_2") != -1:
            if fname.find("version120") != -1:
                return ["test/SpiderMonkey/shell.js", "test/SpiderMonkey/js1_2/version120/shell.js"]
            return ["test/SpiderMonkey/shell.js"]
        elif fname.find("js1_5") != -1:
            if fname.find("Expressions") != -1:
                return ["test/SpiderMonkey/shell.js", "test/SpiderMonkey/js1_5/Expressions/shell.js"]
            return ["test/SpiderMonkey/shell.js"]
        elif fname.find("js1_6") != -1:
            return ["test/SpiderMonkey/shell.js", "test/SpiderMonkey/js1_6/shell.js"]
        elif fname.find("js1_7") != -1:
            return ["test/SpiderMonkey/shell.js", "test/SpiderMonkey/js1_7/shell.js"]
        elif fname.find("js1_8") != -1:
            if fname.find("js1_8_1") != -1:
                if fname.find("jit") != -1:
                    return ["test/SpiderMonkey/shell.js", "test/SpiderMonkey/js1_8_1/shell.js", "test/SpiderMonkey/js1_8_1/jit/shell.js"]
                elif fname.find("strict") != -1:
                    return ["test/SpiderMonkey/shell.js", "test/SpiderMonkey/js1_8_1/shell.js", "test/SpiderMonkey/js1_8_1/strict/shell.js"]
                return ["test/SpiderMonkey/shell.js", "test/SpiderMonkey/js1_8_1/shell.js"]
            elif fname.find("js1_8_5") != -1:
                if fname.find("extensions") != -1:
                    return ["test/SpiderMonkey/shell.js", "test/SpiderMonkey/js1_8_5/shell.js", "test/SpiderMonkey/js1_8_5/extensions/shell.js"]
                elif fname.find("reflect-parse") != -1:
                    return ["test/SpiderMonkey/shell.js", "test/SpiderMonkey/js1_8_5/shell.js", "test/SpiderMonkey/js1_8_5/reflect-parse/shell.js"]
                return ["test/SpiderMonkey/shell.js", "test/SpiderMonkey/js1_8_5/shell.js"]
            return ["test/SpiderMonkey/shell.js", "test/SpiderMonkey/js1_8/shell.js"]
        else:
            return ["test/SpiderMonkey/shell.js"]

    def output_file(self, a_v_m):
        return "test/SpiderMonkey/mozilla." + '.'.join(a_v_m) + ".gen.txt"

class Driver(object):
    def main(self):
        args = sys.argv[1:]

        parser = ArgumentParser()
        (path, options) = parser.parse(args)

        a_v_ms = []
        for a_v_m in options.arch_and_variants_and_mode:
            a_v_ms.append(a_v_m)

        module = importlib.import_module("driver")
        instance = 0
        for suite in options.suite:
            if suite == "stress":
                class_ = getattr(module, "StressReader")
                instance = class_()
            elif suite == "mozilla":
                class_ = getattr(module, "MozillaReader")
                instance = class_()

        def log(f, str):
            print(str)
            f.write(str + '\n')

        idx = 0
        for a_v_m in a_v_ms:
            shell = os.path.join("out", a_v_m[0], a_v_m[1], a_v_m[2], "escargot")
            total = 0
            succ = 0
            fail = 0
            timeout = 0
            idx += 1
            with open(instance.output_file(a_v_m), 'w') as f:
                for tc in instance.list_tests():
                    try:
                        total += 1
                        command = [shell]
                        command = command + instance.mandatory_file(tc)
                        command.append(tc)
                        output = subprocess.check_output(command, timeout=options.timeout)
                        succ += 1
                        log(f, ' '.join(command) + ".... Success")
                    except subprocess.TimeoutExpired as e:
                        timeout += 1
                        log(f, ' '.join(command) + ".... Timeout")
                    except subprocess.CalledProcessError as e:
                        fail += 1
                        log(f, ' '.join(command) + ".... Fail")
                log(f, 'total : ' + str(total))
                log(f, 'succ : ' + str(succ))
                log(f, 'fai : ' + str(fail))
                log(f, 'timeout : ' + str(timeout))

        return True

if __name__ == "__main__":
    sys.exit(Driver().main())
