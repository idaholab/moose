
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
      {"unterminated section", "[hello]"},
      {"unterminated section", "[hello][./]"},
      {"empty  dotslash section name", "[./][]"},
      {"extra section close", "[]"},
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
      {"no whitespace between headers/footers", "[hello][../]"},
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

struct ExpandCase
{
  std::string name;
  std::string input;
  std::string key;
  std::string val;
  hit::Field::Kind kind;
};

TEST(ExpandWalkerTests, All)
{
  ExpandCase cases[] = {
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

  for (size_t i = 0; i < sizeof(cases) / sizeof(ExpandCase); i++)
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
