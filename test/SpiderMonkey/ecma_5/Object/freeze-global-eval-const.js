// |reftest| skip-if(!xulRuntime.shell) -- uses evalcx
/*
 * Any copyright is dedicated to the Public Domain.
 * http://creativecommons.org/licenses/publicdomain/
 */

try {
    evalcx("Object.freeze(this); eval('var q = undefined;')");
} catch (e) {
    assertEq(e.message, "cannot redefine property");
}

reportCompare(0, 0, "don't crash");
