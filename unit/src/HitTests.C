//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "hit.h"
#include "Parser.h"

#include "gtest_include.h"

#include <iostream>
#include <vector>

// TODO:
//
// * don't include trailing newlines in tokens (e.g. unquoted strings, strings, numbers, etc.)
// * build proper nesting structure
// * throw errors for bad/invalid number formats - both during parsing and retrieval (i.e. param
// func)
// * "${}" expressions get tokenized into strings correctly (including nesting, with spaces, etc.)
// * Lots of tests for tree merging
//
//     - existing param gets overridden in dst
//     - non-existing param gets duplicated into dst (not moved)
//     - existing tree gets overridden correctly
//     - non-existing tree gets duplicated into dst (not moved)

struct ValCase
{
  std::string name;
  std::string input;
  std::string key;
  std::string val;
  hit::Field::Kind kind;
};

std::string
strkind(hit::Field::Kind k)
{
  if (k == hit::Field::Kind::String)
    return "String";
  else if (k == hit::Field::Kind::Bool)
    return "Bool";
  else if (k == hit::Field::Kind::Int)
    return "Int";
  else if (k == hit::Field::Kind::Float)
    return "Float";
  else if (k == hit::Field::Kind::None)
    return "None";
  else
    return "Unknown";
}

TEST(HitTests, BraceExpressions)
{
  ValCase cases[] = {
      {"substitute string", "foo=bar boo=${foo}", "boo", "bar", hit::Field::Kind::String},
      {"substitute string explicit",
       "foo=bar boo=${replace foo}",
       "boo",
       "bar",
       hit::Field::Kind::String},
      {"trailing space", "foo=bar boo=${foo} ", "boo", "bar", hit::Field::Kind::String},
      {"substute number", "foo=42 boo=${foo}", "boo", "42", hit::Field::Kind::Int},
      {"multiple replacements",
       "foo=42 boo='${foo} ${foo}'",
       "boo",
       "42 42",
       hit::Field::Kind::String},
      {"nested", "foo=bar [hello] boo='${foo}' []", "hello/boo", "bar", hit::Field::Kind::String},
      {"repl-header-before",
       "src=foo [src] bar='${src}' []",
       "src/bar",
       "foo",
       hit::Field::Kind::String},
      {"repl-header-missing", "[src] bar='${src}' []", "src/bar", "${src}", hit::Field::Kind::None},
      {"repl-header-shadow",
       "[src] bar='${src}' [] src=foo",
       "src/bar",
       "${src}",
       hit::Field::Kind::None},
      {"nested shadow",
       "foo=bar [hello] foo=baz boo='${foo}' []",
       "hello/boo",
       "baz",
       hit::Field::Kind::String},
      {"multi-line brace expression",
       "foo=${raw 4\n"
       "          2\n"
       "     }",
       "foo",
       "42",
       hit::Field::Kind::String},
      {"fparse", "foo=${fparse 40 + 2}\n", "foo", "42", hit::Field::Kind::Float},
      {"fparse-dep-chain",
       "foo=${fparse 42} bar=${fparse foo}",
       "bar",
       "42",
       hit::Field::Kind::Float},
      {"fparse-dep-chain-quoted",
       "foo='${fparse 42}' bar=${fparse foo}",
       "bar",
       "42",
       hit::Field::Kind::Float},
      {"fparse-with-pi", "foo=${fparse cos(pi)}\n", "foo", "-1", hit::Field::Kind::Float},
      {"fparse-with-e", "foo=${fparse log(e)}\n", "foo", "1", hit::Field::Kind::Float},
      {"fparse-with-var", "var=39 foo=${fparse var + 3}", "foo", "42", hit::Field::Kind::Float},
      {"brace-expression-ends-before-newline",
       "foo=${raw 42} bar=23",
       "bar",
       "23",
       hit::Field::Kind::Int},
      {"super-complicated",
       "foo1 = 42\n"
       "foo2 = 43\n"
       "[section1]\n"
       "  num = 1\n"
       "  bar = ${${raw foo ${num}}} # becomes 42\n"
       "[]\n"
       "[section2]\n"
       "  num = 2\n"
       "  bar = ${${raw foo ${num}}}  # becomes 43\n"
       "[]\n"
       "\n"
       "a = ${fparse\n"
       "      ${section1/bar} + foo1 / foo2\n"
       "     }\n",
       "a",
       "42.97674418604651",
       hit::Field::Kind::Float},
  };

  for (size_t i = 0; i < sizeof(cases) / sizeof(ValCase); i++)
  {
    auto test = cases[i];
    hit::Node * root = nullptr;
    try
    {
      root = hit::parse("TEST", test.input);
      hit::BraceExpander exw("TEST");
      hit::RawEvaler raw;
      hit::ReplaceEvaler repl;
      FuncParseEvaler fparse_ev;
      exw.registerEvaler("fparse", fparse_ev);
      exw.registerEvaler("raw", raw);
      exw.registerEvaler("replace", repl);
      root->walk(&exw);
      if (exw.errors.size() > 0 && test.kind != hit::Field::Kind::None)
      {
        for (auto & err : exw.errors)
          FAIL() << "case " << i + 1 << " unexpected error: " << err << "\n";
        continue;
      }
      else if (exw.errors.size() == 0 && test.kind == hit::Field::Kind::None)
      {
        FAIL() << "case " << i + 1 << " missing expected error\n";
        continue;
      }
    }
    catch (std::exception & err)
    {
      FAIL() << "case " << i + 1 << " unexpected error: " << err.what() << "\n";
      continue;
    }

    auto n = root->find(test.key);
    if (!n)
    {
      FAIL() << "case " << i + 1 << " failed to find key '" << test.key << "'\n";
      continue;
    }
    if (n->strVal() != test.val)
    {
      FAIL() << "case " << i + 1 << " wrong value (key=" << test.key << "): got '" << n->strVal()
             << "', want '" << test.val << "'\n";
      continue;
    }

    auto f = dynamic_cast<hit::Field *>(n);
    if (!f)
      FAIL() << "case " << i + 1 << " node type is not NodeType::Field";
    else if (test.kind != hit::Field::Kind::None && f->kind() != test.kind)
      FAIL() << "case " << i + 1 << " wrong kind (key=" << test.key << "): got '"
             << strkind(f->kind()) << "', want '" << strkind(test.kind) << "'\n";
  }
}
