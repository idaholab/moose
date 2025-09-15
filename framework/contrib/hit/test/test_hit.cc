//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "hit/parse.h"
#include "hit/braceexpr.h"

#include "gtest/gtest.h"

#include <sstream>
#include <fstream>
#include <cstdio>

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
  LineWalker(int i, const std::vector<int> & want_lines) : _case(i), _want(want_lines) {};
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
       hit::Field::Kind::String},
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
      {"cosecutive string literal 1", "foo='bar''baz'", "foo", "barbaz", hit::Field::Kind::String},
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
      // numbers with # in front; used to represent issue #'s in test harness
      {"number with #", "issue='#1234'", "issue", "#1234", hit::Field::Kind::String},
      {"multiple numbers with #s",
       "issue='#1234 #5678'",
       "issue",
       "#1234 #5678",
       hit::Field::Kind::String}};

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
      {"preserve quotes preceding blankline", "foo = '42'\n\n", "foo = '42'", 0},
      {"preserve block comment (#10889)",
       "[hello]\n  foo = '42'\n\n  # comment\n  bar = 'baz'\n[]",
       "[hello]\n  foo = '42'\n\n  # comment\n  bar = 'baz'\n[]",
       0},
      {"preserve block comment 2 (#10889)",
       "[hello]\n  foo = '42'\n  # comment\n  bar = 'baz'\n[]",
       "[hello]\n  foo = '42'\n  # comment\n  bar = 'baz'\n[]",
       0},
      {"complex newline render",
       "[section01]\n\n  field01 = 10\n\n\n\n  field02 = '20'\n\n  [section02]"
       "\n\n    field03 = '30 31 32 33'\n\n\n    field04 = 40\n    [section03]"
       "\n\n\n\n\n\n      field05 = \"double 50 quoted 51 string\"\n\n\n    []"
       "\n\n\n    field06 = 60\n\n\n\n  []\n  field07 = '70 71 72 73 74'\n\n[]",
       "[section01]\n\n  field01 = 10\n\n  field02 = '20'\n\n  [section02]\n\n  "
       "  field03 = '30 31 32 33'\n\n    field04 = 40\n    [section03]\n\n      "
       "field05 = \"double 50 quoted 51 string\"\n    []\n\n    field06 = 60\n  "
       "[]\n  field07 = '70 71 72 73 74'\n[]",
       0},
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
    EXPECT_EQ("TESTCASE:1.1: negative value read", std::string(err.what()));
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
    EXPECT_EQ("TESTCASE:1.1: negative value read", std::string(err.what()));
  }
}

// helper for recursively capturing each path and leaf value in a parse tree
void
tree_list(hit::Node * node, std::ostringstream & tree_stream)
{
  // capture node path and value of the parameter if current node is a field
  if (node->type() == hit::NodeType::Field)
  {
    tree_stream << "/" << node->fullpath() << " (" << static_cast<hit::Field *>(node)->val() << ")"
                << " - fname: " << std::setw(31) << std::left << node->filename()
                << " line: " << std::setw(2) << std::right << node->line()
                << " column: " << std::setw(2) << std::right << node->column() << "\n";
  }

  // capture node path and recurse the children if current node is a section
  else if (node->type() == hit::NodeType::Section)
  {
    tree_stream << "/" << std::setw(27) << std::left << node->fullpath()
                << " - fname: " << std::setw(31) << std::left << node->filename()
                << " line: " << std::setw(2) << std::right << node->line()
                << " column: " << std::setw(2) << std::right << node->column() << "\n";

    for (auto child : node->children())
    {
      tree_list(child, tree_stream);
    }
  }
}

// test ability to include external file content from various input contexts
TEST(HitTests, FileIncludeSuccess)
{
  // base input file string includes a file within a block and at root level
  std::string basefile = R"INPUT(
[Block01]
  param01a = value01a
  !include include_param_from_basefile.i
  param01c = value01c
[]
!include include_block_from_basefile.i
[Block03]
  param03a = value03a
  param03b = value03b
  param03c = value03c
[]
)INPUT";

  // write param input to file on disk that is included from base input file
  std::ofstream include_param_from_basefile("include_param_from_basefile.i");
  include_param_from_basefile << R"INPUT(
  param01b = value01b
)INPUT";
  include_param_from_basefile.close();

  // write block input to file on disk that is included from base input file
  std::ofstream include_block_from_basefile("include_block_from_basefile.i");
  include_block_from_basefile << R"INPUT(
[Block02]
  param02a = value02a
  !include include_param_from_included.i
  param02c = value02c
[]
)INPUT";
  include_block_from_basefile.close();

  // write param input to file on disk that is included from an include file
  std::ofstream include_param_from_included("include_param_from_included.i");
  include_param_from_included << R"INPUT(
  param02b = value02b
)INPUT";
  include_param_from_included.close();

  // parse the base input string to consume all contents from included files
  auto root = hit::parse("BASE-STRING", basefile);

  // delete the three files that were put on disk to be parsed from includes
  std::remove("include_block_from_basefile.i");
  std::remove("include_param_from_basefile.i");
  std::remove("include_param_from_included.i");

  // expected content from all include files when the parse tree is rendered
  std::string render_expect = R"INPUT(
[Block01]
  param01a = value01a
  param01b = value01b
  param01c = value01c
[]
[Block02]
  param02a = value02a
  param02b = value02b
  param02c = value02c
[]
[Block03]
  param03a = value03a
  param03b = value03b
  param03c = value03c
[]
)INPUT";

  // check that all file content is included when the parse tree is rendered
  EXPECT_EQ(render_expect, "\n" + root->render() + "\n");

  // expected content from all include files when the parse paths are listed
  std::string tree_expect = R"INPUT(
/                            - fname: BASE-STRING                     line:  2 column:  1
/Block01                     - fname: BASE-STRING                     line:  2 column:  1
/Block01/param01a (value01a) - fname: BASE-STRING                     line:  3 column:  3
/Block01/param01b (value01b) - fname: ./include_param_from_basefile.i line:  2 column:  3
/Block01/param01c (value01c) - fname: BASE-STRING                     line:  5 column:  3
/Block02                     - fname: ./include_block_from_basefile.i line:  2 column:  1
/Block02/param02a (value02a) - fname: ./include_block_from_basefile.i line:  3 column:  3
/Block02/param02b (value02b) - fname: ./include_param_from_included.i line:  2 column:  3
/Block02/param02c (value02c) - fname: ./include_block_from_basefile.i line:  5 column:  3
/Block03                     - fname: BASE-STRING                     line:  8 column:  1
/Block03/param03a (value03a) - fname: BASE-STRING                     line:  9 column:  3
/Block03/param03b (value03b) - fname: BASE-STRING                     line: 10 column:  3
/Block03/param03c (value03c) - fname: BASE-STRING                     line: 11 column:  3
)INPUT";

  // traverse the parse tree recursively capturing paths and terminal values
  std::ostringstream tree_actual;
  tree_list(root, tree_actual);

  // check that all file content is included when the parse paths are listed
  EXPECT_EQ(tree_expect, "\n" + tree_actual.str());
}

// test error check for external file include that has nonexistent file path
TEST(HitTests, FileIncludeMissing)
{
  // base input file string includes a file using a path that does not exist
  std::string basefile = R"INPUT(
[Block01]
  param01a = value01a
  !include missing_file.i
  param01c = value01c
[]
)INPUT";

  // check parsing base input fails with error of including nonexistent file
  try
  {
    hit::parse("TESTCASE", basefile);
    FAIL() << "Exception was not thrown";
  }
  catch (std::exception & err)
  {
    EXPECT_EQ("TESTCASE:4.3: could not find 'missing_file.i'", std::string(err.what()));
  }
}

// test error check for external file include that causes circular reference
TEST(HitTests, FileIncludeCircular)
{
  // base input file string includes existing include file 01 within a block
  std::string basefile = R"INPUT(
[Block01]
  param01a = value01a
  !include include_file_01.i
  param01c = value01c
[]
)INPUT";

  // write include file 01 to disk which includes include file 02 downstream
  std::ofstream include_file_01("include_file_01.i");
  include_file_01 << R"INPUT(
[Block02]
  !include include_file_02.i
[]
)INPUT";
  include_file_01.close();

  // write include file 02 to disk which includes include file 01 circularly
  std::ofstream include_file_02("include_file_02.i");
  include_file_02 << R"INPUT(
[Block03]
  !include include_file_01.i
[]
)INPUT";
  include_file_02.close();

  // check parsing base input fails with error of creating circular includes
  try
  {
    hit::parse("TESTCASE", basefile);
    FAIL() << "Exception was not thrown";
  }
  catch (std::exception & err)
  {
    EXPECT_EQ("./include_file_02.i:3.3: file include would create circular reference "
              "'include_file_01.i'",
              std::string(err.what()));
  }

  // delete two extra files that were put on disk to be parsed from includes
  std::remove("include_file_01.i");
  std::remove("include_file_02.i");
}

// test scenario where nested file include is first element in included file
TEST(HitTests, FileIncludeFirstField)
{
  // base input file defines parameter after all content from included files
  std::string file_a = R"INPUT(
!include fileB.i
param_03 = 30
)INPUT";

  // write input to file on disk that includes another file as first element
  std::ofstream file_b("fileB.i");
  file_b << R"INPUT(
!include fileC.i
param_02 = 20
)INPUT";
  file_b.close();

  // write input to file on disk that another file includes as first element
  std::ofstream file_c("fileC.i");
  file_c << R"INPUT(
param_01 = 10
)INPUT";
  file_c.close();

  // parse the base input string to consume all contents from included files
  auto root = hit::parse("FILE-A", file_a);

  // delete two extra files that were put on disk to be parsed from includes
  std::remove("fileB.i");
  std::remove("fileC.i");

  // expected content from all include files when the parse tree is rendered
  std::string render_expect = R"INPUT(
param_01 = 10
param_02 = 20
param_03 = 30
)INPUT";

  // check that all file content is included when the parse tree is rendered
  EXPECT_EQ(render_expect, "\n" + root->render() + "\n");
}

// test capability that blocks of the same name are merged into single block
TEST(HitTests, BlockMerge)
{
  // input that has multiple blocks of the same name and split child content
  std::string input = R"INPUT(
[Block_01]

  param_01_a = value_01_a
  param_01_b = value_01_b

  [Subblock_01_01]
    param_01_01_a = value_01_01_a
    param_01_01_b = value_01_01_b
  []
[]

[Block_02]

  param_02_a = value_02_a
  param_02_b = value_02_b

  [Subblock_02_01]
    param_02_01_a = value_02_01_a
    param_02_01_b = value_02_01_b
  []
[]

[Block_02]

  param_02_c = value_02_c
  param_02_d = value_02_d

  [Subblock_02_01]
    param_02_01_c = value_02_01_c
    param_02_01_d = value_02_01_d
  []

  [Subblock_02_02]
    param_02_02_a = value_02_02_a
    param_02_02_b = value_02_02_b
  []
[]

[Block_01]

  [Subblock_01_02]
    param_01_02_a = value_01_02_a
    param_01_02_b = value_01_02_b
  []

  param_01_c = value_01_c
  param_01_d = value_01_d
[]

[Block_01]

  param_01_e = value_01_e
  param_01_f = value_01_f

  [Subblock_01_01]
    param_01_01_c = value_01_01_c
    param_01_01_d = value_01_01_d
  []

  param_01_g = value_01_g
  param_01_h = value_01_h
[]
)INPUT";

  // parse input string which merges content from blocks that have same name
  auto root = hit::parse("TESTCASE", input);

  // expected content from all merged blocks when the parse tree is rendered
  std::string render_expect = R"INPUT(
[Block_01]

  param_01_a = value_01_a
  param_01_b = value_01_b

  [Subblock_01_01]
    param_01_01_a = value_01_01_a
    param_01_01_b = value_01_01_b

    param_01_01_c = value_01_01_c
    param_01_01_d = value_01_01_d
  []

  [Subblock_01_02]
    param_01_02_a = value_01_02_a
    param_01_02_b = value_01_02_b
  []

  param_01_c = value_01_c
  param_01_d = value_01_d

  param_01_e = value_01_e
  param_01_f = value_01_f

  param_01_g = value_01_g
  param_01_h = value_01_h
[]

[Block_02]

  param_02_a = value_02_a
  param_02_b = value_02_b

  [Subblock_02_01]
    param_02_01_a = value_02_01_a
    param_02_01_b = value_02_01_b

    param_02_01_c = value_02_01_c
    param_02_01_d = value_02_01_d
  []

  param_02_c = value_02_c
  param_02_d = value_02_d

  [Subblock_02_02]
    param_02_02_a = value_02_02_a
    param_02_02_b = value_02_02_b
  []
[]
)INPUT";

  // check that merging of blocks is correct when the parse tree is rendered
  EXPECT_EQ(render_expect, "\n" + root->render() + "\n");
}

// test ability to override values of parameters when using included inputs
TEST(HitTests, ParamOverrideSuccess)
{
  // base input that includes content from file to be written on disk below
  std::string file_a = R"INPUT(
[Block]
  param_01 :=         value_01_from_file_a
  param_02 :override= value_02_from_file_a
  param_03 =          value_03_from_file_a
  param_04 =          value_04_from_file_a
  param_05 =          value_05_from_file_a
[]
!include file_b.i
)INPUT";

  // write content to file on disk that gets included from base input above
  std::ofstream file_b("file_b.i");
  file_b << R"INPUT(
[Block]
  param_01 =          value_01_from_file_b
  param_02 =          value_02_from_file_b
  param_03 :=         value_03_from_file_b
  param_04 :override= value_04_from_file_b
  param_05 =          value_05_from_file_b
[]
)INPUT";
  file_b.close();

  // parse base input string to also consume all content from included file
  auto root = hit::parse("FILE-A", file_a);

  // delete extra file which was put on disk to be parsed from base include
  std::remove("file_b.i");

  // expected render after parameter conflicts are resolved using overrides
  std::string render_expect = R"INPUT(
[Block]
  param_01 = value_01_from_file_a
  param_02 = value_02_from_file_a
  param_03 = value_03_from_file_b
  param_04 = value_04_from_file_b
  param_05 = value_05_from_file_a
  param_05 = value_05_from_file_b
[]
)INPUT";

  // check override resolution is as expected when parse tree gets rendered
  EXPECT_EQ(render_expect, "\n" + root->render() + "\n");

  // expected origin information of input after override conflicts resolved
  std::string tree_expect = R"INPUT(
/                            - fname: FILE-A                          line:  2 column:  1
/Block                       - fname: FILE-A                          line:  2 column:  1
/Block/param_01 (value_01_from_file_a) - fname: FILE-A                          line:  3 column:  3
/Block/param_02 (value_02_from_file_a) - fname: FILE-A                          line:  4 column:  3
/Block/param_03 (value_03_from_file_b) - fname: ./file_b.i                      line:  5 column:  3
/Block/param_04 (value_04_from_file_b) - fname: ./file_b.i                      line:  6 column:  3
/Block/param_05 (value_05_from_file_a) - fname: FILE-A                          line:  7 column:  3
/Block/param_05 (value_05_from_file_b) - fname: ./file_b.i                      line:  7 column:  3
)INPUT";

  // traverse parse tree recursively to capture origin information of input
  std::ostringstream tree_actual;
  tree_list(root, tree_actual);

  // check parameter origin locations resolved by overrides are as expected
  EXPECT_EQ(tree_expect, "\n" + tree_actual.str());
}

// test error condition of conflicting parameters both specifying overrides
TEST(HitTests, ParamOverrideFailure)
{
  // base input that includes content from file to be written on disk below
  std::string file_a = R"INPUT(
[Block]
  param_01 := value_01_from_file_a
[]
!include file_b.i
)INPUT";

  // write content to file on disk that gets included from base input above
  std::ofstream file_b("file_b.i");
  file_b << R"INPUT(
[Block]
  param_01 :override= value_01_from_file_b
[]
)INPUT";
  file_b.close();

  // expected error if parameter is specified more than once using override
  std::string error_expect = R"INPUT(
FILE-A:3.3: 'Block/param_01' specified more than once with override syntax
)INPUT";

  // parse and expect error due to parameter specified using override twice
  try
  {
    hit::parse("FILE-A", file_a);
    FAIL() << "Exception was not thrown";
  }
  catch (hit::Error & err)
  {
    EXPECT_EQ(error_expect, "\n" + std::string(err.what()) + "\n");
  }

  // delete extra file which was put on disk to be parsed from base include
  std::remove("file_b.i");
}
