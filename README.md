ESCARGOT
========

# Building

    git clone ssh://git@10.251.44.23:7999/escar/escargot.git
    cd escargot
    ./init_third_party.sh
    ./build_third_party.sh
    make [interpreter|jit].[debug|release] -j8
e.g. $ make interpreter.debug

# Running

## Running sunspider
    make run-sunspider

## Running octane
    ./run-octane.sh

## Running test262
    make run-test262 [OPT=**]
e.g. make run-test262 S7.2
[test262:fbba29f](https://github.com/tc39/test262)

## Measuring
    ./measure.sh [escargot | v8 | jsc.interp | jsc.jit] [time | memory]

## Testing JIT
	make check-jit

# Contributing

[Webkit coding style guidelines](https://www.webkit.org/coding/coding-style.html)


