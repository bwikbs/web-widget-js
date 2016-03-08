// escargot-skip: let keyword not supported
/* -*- indent-tabs-mode: nil; js-indent-level: 2 -*- */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

//-----------------------------------------------------------------------------
var BUGNUMBER = 465460;
var summary = 'TM: valueOf in a loop';
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
 
  expect = '11111';

  jit(true);

  (function(d) { for (let j = 0; j < 5; ++j) { actual += ('' + d); } })({valueOf: function()1});

  jit(false);

  reportCompare(expect, actual, summary);

  exitFunc ('test');
}
