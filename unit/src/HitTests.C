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

#include "gtest/gtest.h"

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

struct PassFailCase
{
  std::string name;
  std::string input;
};

TEST(HitTests, FailCases)
{
  PassFailCase cases[] = {
      {"comment in path", "[hello#world] []"},
      {"comment in field", "hello#world=foo"},
      {"invalid path char '='", "[hello=world] []"},
      {"invalid path char '&'", "[hello&world] []"},
      {"invalid path char '['", "[hello[world] []"},
      {"invalid field char '&'", "hello&world=foo"},
      {"invalid field char '['", "hello[world=foo"},
      {"unfinished field", "hello\nfoo=bar"},
      {"unterminated section", "[hello]"},
      {"unterminated section", "[hello][./]"},
      {"extra section close", "[]"},
      {"extra section close 2", "[../]"},
      {"empty  dotslash section name", "[./][]"},
  };

  for (size_t i = 0; i < sizeof(cases) / sizeof(PassFailCase); i++)
  {
    auto test = cases[i];
    EXPECT_THROW(hit::parse("TESTCASE", test.input), hit::Error)
        << "case " << i + 1 << " FAIL (" << test.name << "): parser failed to error on bad input '"
        << test.input << "'";
  }
}

TEST(HitTests, PassCases)
{
  PassFailCase cases[] = {
      {"valid special field chars", "hello_./:<>-+world=foo"},
      {"comment in field", "hello=wo#rld"},
      {"bad number becomes string", "foo=4.2abc"},
      {"empty section close", "[hello] []"},
      {"dotdotslash section close", "[hello] [../]"},
      {"no whitespace between headers/footers", "[hello][../]"},
      {"no whitespace between headers/footers", "[hello][]"},
      {"no whitespace with sections and fields", "[hello][world]foo=bar[]baz=42[]"},
      {"no leading ./ in sub-block", "[hello] [world] [] []"},
  };

  for (size_t i = 0; i < sizeof(cases) / sizeof(PassFailCase); i++)
  {
    auto test = cases[i];
    try
    {
      hit::parse("TESTCASE", test.input);
    }
    catch (hit::Error & err)
    {
      FAIL() << "case " << i + 1 << " FAIL (" << test.name
             << "): unexpected parser error on valid input '" << test.input << "': " << err.what();
    }
  }
}

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

struct ValCase
{
  std::string name;
  std::string input;
  std::string key;
  std::string val;
  hit::Field::Kind kind;
};

TEST(HitTests, ExplodeParentless)
{
  hit::Node * n = new hit::Section("foo/bar");

  try
  {
    n = hit::explode(n);
    EXPECT_EQ("[foo]\n  [bar]\n  []\n[]", n->render());
  }
  catch (std::exception & err)
  {
    FAIL() << "unexpected error: " << err.what();
  }
}

TEST(HitTests, ParseFields)
{
  ValCase cases[] = {
      // types
      {"int", "foo=42", "foo", "42", hit::Field::Kind::Int},
      {"float1", "foo=4.2", "foo", "4.2", hit::Field::Kind::Float},
      {"float2", "foo=.42", "foo", ".42", hit::Field::Kind::Float},
      {"float3", "foo=1e10", "foo", "1e10", hit::Field::Kind::Float},
      {"float4", "foo=e-23", "foo", "e-23", hit::Field::Kind::Float},
      {"float5", "foo=12.345e+67", "foo", "12.345e+67", hit::Field::Kind::Float},
      {"bool-true1", "foo=true", "foo", "true", hit::Field::Kind::Bool},
      {"bool-true2", "foo=yes", "foo", "yes", hit::Field::Kind::Bool},
      {"bool-true3", "foo=on", "foo", "on", hit::Field::Kind::Bool},
      {"bool-case1", "foo=TRUE", "foo", "TRUE", hit::Field::Kind::Bool},
      {"bool-case2", "foo=ON", "foo", "ON", hit::Field::Kind::Bool},
      {"bool-case3", "foo=YeS", "foo", "YeS", hit::Field::Kind::Bool},
      {"bool-false1", "foo=false", "foo", "false", hit::Field::Kind::Bool},
      {"bool-false2", "foo=no", "foo", "no", hit::Field::Kind::Bool},
      {"bool-false3", "foo=off", "foo", "off", hit::Field::Kind::Bool},
      {"string", "foo=bar", "foo", "bar", hit::Field::Kind::String},
      {"string-almost-float1", "foo=1e23.3", "foo", "1e23.3", hit::Field::Kind::String},
      {"string-almost-float2", "foo=1a23.3", "foo", "1a23.3", hit::Field::Kind::String},
      {"string-almost-float3", "foo=1.2.3", "foo", "1.2.3", hit::Field::Kind::String},
      {"string-almost-float4", "foo=1e2e3", "foo", "1e2e3", hit::Field::Kind::String},

      // quotes and escaping
      {"quotes", "foo='bar'", "foo", "bar", hit::Field::Kind::String},
      {"doublequotes", "foo=\"bar\"", "foo", "bar", hit::Field::Kind::String},
      {"quotes_quotes", "foo='\\'bar\\''", "foo", "'bar'", hit::Field::Kind::String},
      {"quotes_doublequotes", "foo='\"bar\"'", "foo", "\"bar\"", hit::Field::Kind::String},
      {"doublequotes_doublequotes",
       "foo=\"\\\"bar\\\"\"",
       "foo",
       "\"bar\"",
       hit::Field::Kind::String},

      // misc
      {"valid special field chars",
       "hello_./:<>-+world=foo",
       "hello_./:<>-+world",
       "foo",
       hit::Field::Kind::String},
      {"ignore leading spaces 1", "foo=    bar", "foo", "bar", hit::Field::Kind::String},
      {"ignore leading spaces 2", "foo=     \t42", "foo", "42", hit::Field::Kind::Int},
      {"ignore trailing spaces", "foo=bar\t   ", "foo", "bar", hit::Field::Kind::String},
      {"ignore unknown escapes",
       "foo='hello \\my nam\\e is joe'",
       "foo",
       "hello \\my nam\\e is joe",
       hit::Field::Kind::String},
      {"no escaped newline",
       "foo='hello\\nworld'",
       "foo",
       "hello\\nworld",
       hit::Field::Kind::String},
  };

  for (size_t i = 0; i < sizeof(cases) / sizeof(ValCase); i++)
  {
    auto test = cases[i];
    auto root = hit::parse("TEST", test.input);
    ExpandWalker exw("TEST");
    root->walk(&exw);
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
    else if (f->kind() != test.kind)
      FAIL() << "case " << i + 1 << " wrong kind (key=" << test.key << "): got '"
             << strkind(f->kind()) << "', want '" << strkind(test.kind) << "'\n";
  }
}
TEST(ExpandWalkerTests, All)
{
  ValCase cases[] = {
      {"substitute string", "foo=bar boo=${foo}", "boo", "bar", hit::Field::Kind::String},
      {"substute number", "foo=42 boo=${foo}", "boo", "42", hit::Field::Kind::Int},
      {"multiple replacements",
       "foo=42 boo=${foo} ${foo}",
       "boo",
       "42 42",
       hit::Field::Kind::String},
      {"nested", "foo=bar [hello] boo='${foo}' []", "hello/boo", "bar", hit::Field::Kind::String},
      {"repl-header-before",
       "src=foo [src] bar='${src}' []",
       "src/bar",
       "foo",
       hit::Field::Kind::String},
      {"repl-header-missing",
       "[src] bar='${src}' []",
       "src/bar",
       "${src}",
       hit::Field::Kind::String},
      {"repl-header-shadow",
       "[src] bar='${src}' [] src=foo",
       "src/bar",
       "${src}",
       hit::Field::Kind::String},
      {"nested shadow",
       "foo=bar [hello] foo=baz boo='${foo}' []",
       "hello/boo",
       "baz",
       hit::Field::Kind::String},
  };

  for (size_t i = 0; i < sizeof(cases) / sizeof(ValCase); i++)
  {
    auto test = cases[i];
    auto root = hit::parse("TEST", test.input);
    ExpandWalker exw("TEST");
    root->walk(&exw);
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
    else if (f->kind() != test.kind)
      FAIL() << "case " << i + 1 << " wrong kind (key=" << test.key << "): got '"
             << strkind(f->kind()) << "', want '" << strkind(test.kind) << "'\n";
  }
}

TEST(HitTests, RenderParentlessSection)
{
  auto n = new hit::Section("mypath");
  try
  {
    std::string got = n->render();
    EXPECT_EQ("[mypath]\n[]", got);
  }
  catch (std::exception & err)
  {
    FAIL() << "failed with unexpected error: " << err.what();
  }
}

TEST(HitTests, RenderSubsection)
{
  auto root = hit::parse("TESTCASE", "[hello][world]foo=42[][]");
  auto n = root->find("hello/world");
  try
  {
    n->render();
    std::string got = n->render();
    EXPECT_EQ("[world]\n  foo = 42\n[]", n->render());
  }
  catch (std::exception & err)
  {
    FAIL() << "failed with unexpected error: " << err.what();
  }
}

struct RenderCase
{
  std::string name;
  std::string input;
  std::string output;
};

TEST(HitTests, RenderCases)
{
  RenderCase cases[] = {
      {"root level fields", "foo=bar boo=far", "foo = bar\nboo = far"},
      {"single section", "[foo]bar=baz[../]", "[foo]\n  bar = baz\n[../]"},
      {"remove leading newline", "\n[foo]bar=baz[../]", "[foo]\n  bar = baz\n[../]"},
      {"preserve consecutive newline", "[foo]\n\nbar=baz[../]", "[foo]\n\n  bar = baz\n[../]"},
  };

  for (size_t i = 0; i < sizeof(cases) / sizeof(RenderCase); i++)
  {
    auto test = cases[i];
    hit::Node * root = nullptr;
    std::string got;
    try
    {
      root = hit::parse("TESTCASE", test.input);
      got = root->render();
    }
    catch (std::exception & err)
    {
      FAIL() << "case " << i + 1 << " FAIL (" << test.name << "): unexpected error: " << err.what();
    }
    EXPECT_EQ(test.output, got) << "case " << i + 1 << " FAIL";
  }
}

TEST(HitTests, MergeTree)
{
  auto root1 = hit::parse("TESTCASE", "[foo]bar=42[]");
  auto root2 = hit::parse("TESTCASE", "foo/baz/boo=42");
  hit::explode(root2);
  hit::merge(root2, root1);
  EXPECT_EQ("[foo]\n  bar = 42\n  [baz]\n    boo = 42\n  []\n[]", root1->render());
}
