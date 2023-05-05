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
      {"missing string", "[hello] foo = []"},
      {"missing string 2", "[hello] foo = \n bar = 42[]"},
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
      {"mismatched consecutive string literal quotes", "foo='bar'\"baz\""},
  };

  for (size_t i = 0; i < sizeof(cases) / sizeof(PassFailCase); i++)
  {
    auto test = cases[i];
    EXPECT_THROW(hit::parse("TESTCASE", test.input), hit::Error)
        << "case " << i + 1 << " FAIL (" << test.name << "): parser failed to error on bad input '"
        << test.input << "'";
  }
}

struct LineCase
{
  std::string input;
  std::vector<int> line_nums;
};

class LineWalker : public hit::Walker
{
public:
  LineWalker(int i, const std::vector<int> & want_lines) : _case(i), _want(want_lines){};
  virtual void
  walk(const std::string & fullpath, const std::string & /*nodepath*/, hit::Node * n) override
  {
    if (n->type() == hit::NodeType::Blank || fullpath == "")
      return;

    if (_count >= _want.size())
      FAIL() << "case " << _case + 1 << " has more nodes than expected";
    EXPECT_EQ(_want[_count], n->line())
        << "case " << _case + 1 << " node " << _count + 1 << " (" << fullpath
        << ") has wrong line: want " << _want[_count] << ", got " << n->line();
    _count++;
  }

private:
  int _case;
  std::vector<int> _want;
  size_t _count = 0;
};

TEST(HitTests, LineNumbers)
{
  // list of expected line numbers for nodes starts at line 1 and skips root and blankline nodes
  LineCase cases[] = {
      {"[hello] foo='bar'\n\n\n boo='far'\n\n[]", {1, 1, 4}},
      {"[hello]\n  foo='bar'\n[]\n[goodbye]\n  boo=42\n[]", {1, 2, 4, 5}},
      {"[hello/bar]\n  foo=42\n[]", {1, 1, 2}},
      {"[hello]\n\n # comment\n foo='bar' 'baz' # another comment\n\nboo=42[]", {1, 3, 4, 4, 6}}};

  for (size_t i = 0; i < sizeof(cases) / sizeof(LineCase); i++)
  {
    auto test = cases[i];
    try
    {
      std::unique_ptr<hit::Node> root(hit::parse("TESTCASE", test.input));
      hit::explode(root.get());
      LineWalker w(i, test.line_nums);
      root->walk(&w, hit::NodeType::All);
    }
    catch (hit::Error & err)
    {
      FAIL() << "case " << i + 1 << " FAIL: unexpected parser error on valid input '" << test.input
             << "': " << err.what();
    }
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
      {"consecutive string literals", "foo='bar''baz'"},
      {"no infinite loop", "foo='bar'\n\n "},
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

TEST(HitTests, ParseFields){
    ValCase cases[] = {
        // types
        {"int", "foo=42", "foo", "42", hit::Field::Kind::Int},
        {"float1", "foo=4.2", "foo", "4.2", hit::Field::Kind::Float},
        {"float2", "foo=.42", "foo", ".42", hit::Field::Kind::Float},
        // Previously, the HIT lexer designated "e-23" below as a 'float' from the text pattern
        // alone but the MOOSE string-to-float conversion logic does not support this no
        // coefficient syntax so even through the kind method called this a 'float', retrieving
        // it as a float would fail however WASP-HIT reuses the convert and retrieve logic for
        // consistent field categorization making "foo=e-23" be designated now as a 'string'
        {"float3", "foo=1e10", "foo", "1e10", hit::Field::Kind::Float},
        {"float4",
         "foo=e-23",
         "foo",
         "e-23", // why do we even support this?
#ifdef WASP_ENABLED
         hit::Field::Kind::String},
#else
         hit::Field::Kind::Float},
#endif
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
        {"left-bracket-after-number", "[hello]foo=42[]", "hello/foo", "42", hit::Field::Kind::Int},
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
        {"cosecutive string literal 1",
         "foo='bar''baz'",
         "foo",
         "barbaz",
         hit::Field::Kind::String},
        {"cosecutive string literal 2",
         "foo='bar'\n\n'baz'",
         "foo",
         "barbaz",
         hit::Field::Kind::String},
        {"path-normalize-find #12313",
         "[foo][/bar]baz=42[][]",
         "foo/bar/baz",
         "42",
         hit::Field::Kind::Int},

    };

for (size_t i = 0; i < sizeof(cases) / sizeof(ValCase); i++)
{
  auto test = cases[i];
  auto root = hit::parse("TEST", test.input);
  hit::BraceExpander exw;
  root->walk(&exw);
  auto n = root->find(test.key);
  if (!n)
  {
    FAIL() << "case " << i + 1 << " (" << test.name << ") failed to find key '" << test.key
           << "'\n";
    continue;
  }
  if (n->strVal() != test.val)
  {
    FAIL() << "case " << i + 1 << " (" << test.name << ") wrong value (key=" << test.key
           << "): got '" << n->strVal() << "', want '" << test.val << "'\n";
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
TEST(HitTests, BraceExpressions){
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
        {"repl-header-missing",
         "[src] bar='${src}' []",
         "src/bar",
         "${src}",
         hit::Field::Kind::None},
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
        // WASP-HIT evaluates the brace expression below to "foo=42" and
        // designates 'int' as the kind
        {"multi-line brace expression",
         "foo=${raw 4\n"
         "          2\n"
         "     }",
         "foo",
         "42",
#ifdef WASP_ENABLED
         hit::Field::Kind::Int},
#else
         hit::Field::Kind::String},
#endif
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
        {"multi-line value",
         "foo = '1 2 3\n"
         "4 5 6\n"
         "7 8 9'\n"
         "boo = ${foo}",
         "boo",
         "1 2 3\n4 5 6\n7 8 9",
         hit::Field::Kind::String},
    };

for (size_t i = 0; i < sizeof(cases) / sizeof(ValCase); i++)
{
  auto test = cases[i];
  hit::Node * root = nullptr;
  try
  {
    root = hit::parse("TEST", test.input);
    hit::BraceExpander exw;
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
  int maxlen;
};

TEST(HitTests, RenderCases)
{
  RenderCase cases[] = {
      {"root level fields", "foo=bar boo=far", "foo = bar\nboo = far", 0},
      {"single section", "[foo]bar=baz[../]", "[foo]\n  bar = baz\n[../]", 0},
      {"remove leading newline", "\n[foo]bar=baz[../]", "[foo]\n  bar = baz\n[../]", 0},
      {"preserve consecutive newline", "[foo]\n\nbar=baz[../]", "[foo]\n\n  bar = baz\n[../]", 0},
      {"reflow long string",
       "foo=\"hello my name is joe and I work in a button factory\"",
       "foo = \"hello my name is joe \"\n      \"and I work in a \"\n      \"button factory\"",
       28},
      {"don't reflow single quoted long string",
       "foo='hello my name is joe and I work in a button factory'",
       "foo = 'hello my name is joe and I work in a button factory'",
       28},
      {"don't reflow unquoted string", "foo=unquotedstring", "foo = unquotedstring", 5},
      {"don't reflow single quoted string", "foo='longstring'", "foo = 'longstring'", 12},
      {"reflow double quoted string", "foo=\"longstring\"", "foo = \"longst\"\n      \"ring\"", 12},
      {"reflow pre-broken strings",
       "foo='why'\n' separate '  'strings?'",
       "foo = 'why separate strings?'",
       0},
  // WASP-HIT preserves quotes before blank lines as tested below, but blank lines are rendered
  // prior to the next non-blank node with blank lines that trail all content not being emitted
  // so a newline was trimmed from the expected output
#ifdef WASP_ENABLED
      {"preserve quotes preceding blankline", "foo = '42'\n\n", "foo = '42'", 0},
#else
      {"preserve quotes preceding blankline", "foo = '42'\n\n", "foo = '42'\n", 0},
#endif
      {"preserve block comment (#10889)",
       "[hello]\n  foo = '42'\n\n  # comment\n  bar = 'baz'\n[]",
       "[hello]\n  foo = '42'\n\n  # comment\n  bar = 'baz'\n[]",
       0},
      {"preserve block comment 2 (#10889)",
       "[hello]\n  foo = '42'\n  # comment\n  bar = 'baz'\n[]",
       "[hello]\n  foo = '42'\n  # comment\n  bar = 'baz'\n[]",
       0},
#ifdef WASP_ENABLED
      {"complex newline render",
       "[section01]\n\n  field01 = 10\n\n\n\n  field02 = '20'\n\n  [section02]"
       "\n\n    field03 = '30 31 32 33'\n\n\n    field04 = 40\n    [section03]"
       "\n\n\n\n\n\n      field05 = \"double 50 quoted 51 string\"\n\n\n    []"
       "\n\n\n    field06 = 60\n\n\n\n  []\n  field07 = '70 71 72 73 74'\n\n[]",
       "[section01]\n\n  field01 = 10\n\n\n\n  field02 = '20'\n\n  [section02]"
       "\n\n    field03 = '30 31 32 33'\n\n\n    field04 = 40\n    [section03]"
       "\n\n\n\n\n\n      field05 = \"double 50 quoted 51 string\"\n\n\n    []"
       "\n\n\n    field06 = 60\n\n\n\n  []\n  field07 = '70 71 72 73 74'\n\n[]",
       0},
#endif
  };

  for (size_t i = 0; i < sizeof(cases) / sizeof(RenderCase); i++)
  {
    auto test = cases[i];
    hit::Node * root = nullptr;
    std::string got;
    try
    {
      root = hit::parse("TESTCASE", test.input);
      got = root->render(0, "  ", test.maxlen);
    }
    catch (std::exception & err)
    {
      FAIL() << "case " << i + 1 << " FAIL (" << test.name << "): unexpected error: " << err.what();
    }
    EXPECT_EQ(test.output, got) << "case " << i + 1 << " FAIL (" << test.name << ")";
  }
}

TEST(HitTests, MergeTree)
{
  {
    auto root1 = hit::parse("TESTCASE", "[foo]bar=42[]");
    auto root2 = hit::parse("TESTCASE", "foo/baz/boo=42");
    hit::explode(root2);
    hit::merge(root2, root1);
    EXPECT_EQ("[foo]\n  bar = 42\n  [baz]\n    boo = 42\n  []\n[]", root1->render());
  }

  {
    auto root1 = hit::parse("TESTCASE", "foo/bar=baz");
    auto root2 = hit::parse("TESTCASE", "foo/bar=42");
    hit::merge(root2, root1);
    auto n = root1->find("foo/bar");
    auto f = dynamic_cast<hit::Field *>(n);
    if (!f)
      FAIL() << "merge case node type is not NodeType::Field";

    // Make sure that the the from type overrides the into type on merge
    else if (f->kind() != hit::Field::Kind::Int)
      FAIL() << "merge case kind type is not overridden (string)";
  }
}

TEST(HitTests, Clone)
{
  auto root1 = hit::parse("TESTCASE", "[foo][bar]baz=42[][]");
  hit::Section root2("");
  hit::Section root3("");
  root2.addChild(root1->children()[0]->children()[0]->children()[0]->clone());
  root3.addChild(root1->children()[0]->children()[0]->children()[0]->clone(true));

  EXPECT_EQ("baz = 42", root2.render());
  EXPECT_EQ("foo/bar/baz = 42", root3.render());
}

TEST(HitTests, GatherParamWalker)
{
  std::vector<std::pair<std::string, std::map<std::string, std::string>>> tests = {
      {"[foo]bar=42[][baz][qux]quux=1[][]", {{"foo/bar", "42"}, {"baz/qux/quux", "1"}}},
      {"foo='123'\nbar=baz", {{"foo", "123"}, {"bar", "baz"}}}};

  for (const auto & test : tests)
  {
    auto root = hit::parse("TESTCASE", test.first);
    hit::GatherParamWalker::ParamMap params;
    hit::GatherParamWalker walker(params);
    root->walk(&walker);

    if (test.second.size() != params.size())
      FAIL() << "case " << test.first << " failed.";

    auto it1 = test.second.begin();
    auto it2 = params.begin();
    for (; it1 != test.second.end() && it2 != params.end(); ++it1, ++it2)
      if (it1->first != it2->first || it1->second != it2->second->strVal())
        FAIL() << "case " << test.first << " failed.";
  }
}

TEST(HitTests, RemoveParamWalker)
{
  std::vector<std::array<std::string, 3>> tests = {
      {"[foo][bar]baz=qux\nlorem=ipsum[][]",
       "foo/bar/baz=qux",
       "[foo]\n  [bar]\n    lorem = ipsum\n  []\n[]"},
      {"[foo][bar]baz=qux[][]", "foo/bar/baz=qux", "[foo]\n  [bar]\n  []\n[]"}};

  for (const auto & test : tests)
  {
    auto root1 = hit::parse("TESTCASE", test[0]);
    auto root2 = hit::parse("TESTCASE", test[1]);

    hit::GatherParamWalker::ParamMap params;
    hit::GatherParamWalker gather_walker(params);
    hit::RemoveParamWalker remove_walker(params);

    root2->walk(&gather_walker);
    root1->walk(&remove_walker);

    EXPECT_EQ(root1->render(), test[2]);
  }
}

TEST(HitTests, RemoveEmptySectionWalker)
{
  std::vector<std::array<std::string, 2>> tests = {
      {"[foo][bar]lorem=ipsum[][]", "[foo]\n  [bar]\n    lorem = ipsum\n  []\n[]"},
      {"[foo][bar][][]", ""},
      {"[foo][bar]\n\n\n[][]", ""},
      {"[foo][bar]\n# comment\n\n[][]", ""}};

  for (const auto & test : tests)
  {
    auto root = hit::parse("TESTCASE", test[0]);

    hit::RemoveEmptySectionWalker walker;
    root->walk(&walker);

    EXPECT_EQ(root->render(), test[1]);
  }
}

TEST(HitTests, Formatter)
{
  RenderCase cases[] = {
      {"[format]line_length=100[]",
       "foo='line longer than 20 characters'",
       "foo = 'line longer than 20 characters'",
       0},
      {"[format]line_length=20[]",
       "foo=\"line longer than 20 characters\"",
       "foo = \"line longer \"\n      \"than 20 \"\n      \"characters\"",
       0},
      {"[format]canonical_section_markers=true[]", "[./foo][../]", "[foo]\n[]", 0},
      {"[format]canonical_section_markers=false[]", "[./foo][../]", "[./foo]\n[../]", 0},
      {"[format]indent_string='    '[]", "[foo]bar=42[]", "[foo]\n    bar = 42\n[]", 0},
      {"[format]indent_string='      '[]", "[foo]bar=42[]", "[foo]\n      bar = 42\n[]", 0},
  };

  for (size_t i = 0; i < sizeof(cases) / sizeof(RenderCase); i++)
  {
    auto test = cases[i];
    std::string got;
    try
    {
      hit::Formatter fmter("STYLE", test.name);
      got = fmter.format("TESTCASE", test.input);
    }
    catch (std::exception & err)
    {
      FAIL() << "case " << i + 1 << " FAIL (" << test.name << "): unexpected error: " << err.what();
    }
    EXPECT_EQ(test.output, got) << "case " << i + 1 << " FAIL (" << test.name << ")";
  }
}

struct SortCase
{
  struct Pattern
  {
    std::string pattern;
    std::vector<std::string> order;
  };
  std::string name;
  std::string input;
  std::string want;
  std::vector<Pattern> patterns;
};

TEST(HitTests, Formatter_sorting)
{
  // clang-format off
  SortCase cases[] = {
      {
        "name",
        "order = bar outof = foo",
        "outof = foo\norder = bar",
        {
           {"", {"outof", "order"}},
           {"pattern2", {"param1", "param2"}}
        }
      }, {
        "name",
        "order = bar outof = foo",
        "order = bar\noutof = foo",
        {
        }
      }
  };
  // clang-format on

  for (size_t i = 0; i < sizeof(cases) / sizeof(SortCase); i++)
  {
    auto test = cases[i];
    std::string got;

    hit::Formatter fmter;
    for (auto & pattern : test.patterns)
      fmter.addPattern(pattern.pattern, pattern.order);

    try
    {
      got = fmter.format("TESTCASE", test.input);
    }
    catch (std::exception & err)
    {
      FAIL() << "case " << i + 1 << " FAIL (" << test.name << "): unexpected error: " << err.what();
    }
    EXPECT_EQ(test.want, got) << "case " << i + 1 << " FAIL (" << test.name << ")";
  }
}

TEST(HitTests, unsigned_int)
{
  hit::Node * root = nullptr;
  try
  {
    root = hit::parse("TESTCASE", "par = -3");
    root->param<unsigned int>("par");
    FAIL() << "Exception was not thrown";
  }
  catch (std::exception & err)
  {
    EXPECT_EQ("negative value read from file 'TESTCASE' on line 1", std::string(err.what()));
  }
}

TEST(HitTests, vector_unsigned_int)
{
  hit::Node * root = nullptr;
  try
  {
    root = hit::parse("TESTCASE", "par = '-3 0 1'");
    root->param<std::vector<unsigned int>>("par");
    FAIL() << "Exception was not thrown";
  }
  catch (std::exception & err)
  {
    EXPECT_EQ("negative value read from file 'TESTCASE' on line 1", std::string(err.what()));
  }
}
