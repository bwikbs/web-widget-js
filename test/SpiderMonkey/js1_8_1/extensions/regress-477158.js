// escargot-skip: let keyword & for each..in statement not supported
/* -*- indent-tabs-mode: nil; js-indent-level: 2 -*- */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

//-----------------------------------------------------------------------------
var BUGNUMBER = 477158;
var summary = 'Do not assert: v == JSVAL_TRUE || v == JSVAL_FALSE';
var actual = '';
var expect = '';

//-----------------------------------------------------------------------------
test();
//-----------------------------------------------------------------------------

function test()
{
  enterFunc ('test');
  printBugNumber(BUGNUMBER);
  printStatus (summary);

  jit(true);

  x = 0;
  x = x.prop;
  for each (let [] in ['', '']) { switch(x) { default: (function(){}); } };

  jit(false);

  reportCompare(expect, actual, summary);

  exitFunc ('test');
}
