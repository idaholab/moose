
#include "hit.h"

#include "gtest/gtest.h"

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
  std::vector<PassFailCase> cases = {
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

  for (size_t i = 0; i < cases.size(); i++)
  {
    auto test = cases[i];
    EXPECT_THROW(hit::parse("TESTCASE", test.input), hit::Error)
        << "case " << i + 1 << " FAIL (" << test.name << "): parser failed to error on bad input '"
        << test.input << "'";
  }
}

TEST(HitTests, PassCases)
{
  std::vector<PassFailCase> cases = {
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

  for (size_t i = 0; i < cases.size(); i++)
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
