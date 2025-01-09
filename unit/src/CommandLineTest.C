//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "gtest_include.h"

#include "CommandLine.h"
#include "InputParameters.h"
#include "MooseEnum.h"

#include <type_traits>

TEST(CommandLineTest, parse)
{
  const auto test_parse =
      [](const std::vector<std::string> & args,
         const std::vector<std::pair<std::string, std::string>> & expect_entries,
         const std::vector<std::optional<std::string>> & expect_subapps = {})
  {
    const auto first_arg = "/path/to/exe";

    CommandLine cl;
    cl.addArgument(first_arg);
    for (const auto & arg : args)
      cl.addArgument(arg);
    cl.parse();

    const auto & entries = std::as_const(cl).getEntries();
    if (args.size())
      ASSERT_EQ(entries.size() - 1, expect_entries.size());
    else
      ASSERT_EQ(entries.size(), 1);

    std::size_t i = 0;
    for (auto & entry : entries)
    {
      const auto & name = entry.name;
      const auto & value = entry.value;

      if (i == 0)
      {
        EXPECT_EQ(name, first_arg);
        EXPECT_FALSE(value);
        ++i;
        continue;
      }

      const auto & [expect_name, expect_value] = expect_entries[i - 1];

      EXPECT_EQ(name, expect_name);
      if (value)
      {
        if (entry.raw_args[0].find("=") != std::string::npos)
          EXPECT_EQ(*entry.value_separator, std::string("="));
        else
          EXPECT_EQ(*entry.value_separator, std::string(" "));
        EXPECT_EQ(*value, expect_value);
      }
      else
      {
        EXPECT_FALSE(entry.value_separator);
        EXPECT_EQ(std::string(), expect_value);
      }
      if (expect_subapps.size() > i - 1 && expect_subapps[i - 1])
      {
        EXPECT_TRUE(entry.subapp_name);
        EXPECT_EQ(*expect_subapps[i - 1], *entry.subapp_name);
      }
      else
        EXPECT_FALSE(entry.subapp_name);

      ++i;
    }
  };

  test_parse({}, {});
  test_parse({"-foo"}, {{"-foo", ""}});
  test_parse({"-foo=bar"}, {{"-foo", "bar"}});
  test_parse({"-foo", "bar"}, {{"-foo", "bar"}});
  test_parse({"-foo", "    bar baz   bang  "}, {{"-foo", "bar baz bang"}});
  test_parse({"--foo", "bar", "baz", "bang"}, {{"--foo", "bar baz bang"}});
  test_parse({"--foo"}, {{"--foo", ""}});
  test_parse({"--foo=bar"}, {{"--foo", "bar"}});
  test_parse({"--foo", "bar"}, {{"--foo", "bar"}});
  test_parse({"--foo", "bar baz bang"}, {{"--foo", "bar baz bang"}});
  test_parse({"foo=bar"}, {{"foo", "bar"}});
  test_parse({"-foo", "-bar"}, {{"-foo", ""}, {"-bar", ""}});
  test_parse({"-foo", "--bar=foo", "test/something=123", "--bang", "big   bada  boom", "--cool"},
             {{"-foo", ""},
              {"--bar", "foo"},
              {"test/something", "123"},
              {"--bang", "big bada boom"},
              {"--cool", ""}});
  test_parse({"--required-capabilities", "slepc>=1 petsc<=2"},
             {{"--required-capabilities", ""}, {"slepc>", "1 petsc<=2"}});
  test_parse({"--required-capabilities=slepc>=1 petsc<=2"},
             {{"--required-capabilities", "slepc>=1 petsc<=2"}});
  test_parse({"--foo", "sub0:Some/value=1", "sub:subb:Another/value=1"},
             {{"--foo", ""}, {"Some/value", "1"}, {"subb:Another/value", "1"}},
             {{}, {"sub0"}, {"sub"}});
}

TEST(CommandLineTest, parseHIT)
{
  const auto test = [](const std::string & arg,
                       const std::string & path = "",
                       const std::string & value = "",
                       const std::string & subapp_name = "")
  {
    CommandLine cl;
    cl.addArgument(arg);
    cl.parse();

    const auto & entries = std::as_const(cl).getEntries();

    ASSERT_EQ(entries.size(), 1);
    const auto & entry = *entries.begin();

    ASSERT_EQ(entry.name, path);
    if (subapp_name.size())
    {
      ASSERT_TRUE(entry.subapp_name);
      ASSERT_EQ(*entry.subapp_name, subapp_name);
    }
    else
      ASSERT_FALSE(entry.subapp_name);
    ASSERT_TRUE(entry.value);
    ASSERT_EQ(*entry.value, value);
    ASSERT_TRUE(entry.value_separator);
    ASSERT_EQ(*entry.value_separator, "=");
    ASSERT_EQ(entry.raw_args.size(), 1);
    ASSERT_EQ(entry.raw_args[0], arg);
    ASSERT_FALSE(entry.used);
    ASSERT_FALSE(entry.global);
    ASSERT_TRUE(entry.hit_param);
  };

  test("foo/bar=123", "foo/bar", "123");
  test("foo=123", "foo", "123");
  test("baz=", "baz", "");
  test("Some/value=  1; 2;   3; 4", "Some/value", "1; 2; 3; 4");
  test("LOUD=!!!", "LOUD", "!!!");
  test("sub:foo/bar=123", "foo/bar", "123", "sub");
  test("sub0:baz/bang=abcd", "baz/bang", "abcd", "sub0");
  test("sub:another:value=zzz", "another:value", "zzz", "sub");
  test("sub:another:yetanother:value=", "another:yetanother:value", "", "sub");
  test("sub:vector_value=\"1 2 3 4\"", "vector_value", "\"1 2 3 4\"", "sub");
  test("single_quoted_value=' a b c'", "single_quoted_value", "' a b c'");

  const auto test_not = [](const std::vector<std::string> & args)
  {
    CommandLine cl;
    cl.addArguments(args);
    cl.parse();

    for (const auto & entry : std::as_const(cl).getEntries())
      ASSERT_FALSE(entry.hit_param);
  };

  test_not({"--abcd", "1234", "-foo"});
  test_not({"-some_value=1234"});
}

TEST(CommandLineTest, parseMultiAppDashedOption)
{
  const auto test = [](const std::string & arg)
  {
    CommandLine cl;
    cl.addArgument(arg);
    try
    {
      cl.parse();
      FAIL();
    }
    catch (const std::exception & err)
    {
      ASSERT_EQ(std::string(err.what()),
                "The MultiApp command line argument '" + arg +
                    "' sets a command line option.\nMultiApp command line arguments can only be "
                    "used for setting HIT parameters.");
    }
  };

  test("sub:-foo=val");
  test("baz0:--foo=val");
  test("bang1:--distributed-mesh");
  test("sub:--foo");
  test("sub:-foo");
  test("sub1:--cool-vector=1 2 3 4");
}

TEST(CommandLineTest, populate)
{
  const auto test_populate = [](const auto value_type,
                                const auto & without_default_value,
                                const auto & required_value,
                                const auto & default_value,
                                const auto & set_default_value)
  {
    using type = typename std::remove_const<
        typename std::remove_reference<decltype(value_type)>::type>::type;

    const auto to_string = [](const auto & value) -> std::string
    {
      std::stringstream ss;
      if constexpr (std::is_same_v<const std::vector<std::string> &, decltype(value)>)
      {
        for (const auto & entry : value)
          ss << entry << " ";
      }
      else
        ss << value;
      return ss.str();
    };

    const auto starts_with_dash = [&to_string](const auto & value)
    { return to_string(value).rfind("-", 0) == 0; };

    constexpr bool is_bool = std::is_same_v<type, bool>;

    InputParameters params = emptyInputParameters();
    params.addCommandLineParam<type>(
        "without_default", "-without-default --without-default", "Doc1");
    // bool doesn't support required or with default
    if constexpr (!is_bool)
    {
      params.addCommandLineParam<type>("required", "-required --required", "Doc2");
      params.addCommandLineParam<type>(
          "with_default", "-with-default --with-default", default_value, "Doc3");
    }
    // Avoid unused params for bool type
    else
    {
      (void)required_value;
      (void)default_value;
      (void)set_default_value;
    }

    // Test setting all of the parameters with "--value=bar" and "--value bar"
    for (const auto with_equals : std::vector<bool>({true, false}))
    {
      // No --value= for a boolean parameter
      if constexpr (is_bool)
        if (with_equals)
          continue;

      // Test setting all of the parameters with "-value" and "--value"
      for (const bool double_dash : std::vector<bool>({false, true}))
      {
        const std::string prefix = double_dash ? "--" : "-";

        CommandLine cl;
        cl.addArgument("/path/to/exe");

        // Bool just has a single name and no value
        if constexpr (is_bool)
        {
          cl.addArgument(prefix + "without-default");
        }
        // Everything else has a value
        else
        {
          // Adds an argument with -/-- and with and without the equals
          const auto add_argument = [&cl, &with_equals, &to_string, &starts_with_dash, &prefix](
                                        const auto & name, const auto & value)
          {
            if (with_equals)
              cl.addArgument(prefix + name + "=" + to_string(value));
            // Without an equals, we can't add anything that starts with a -
            else if (!starts_with_dash(value))
            {
              cl.addArgument(prefix + name);
              cl.addArgument(to_string(value));
            }
          };

          add_argument("without-default", without_default_value);
          add_argument("required", required_value);
          add_argument("with-default", set_default_value);
        }

        cl.parse();

        auto params_copy = params;
        cl.populateCommandLineParams(params_copy);

        if (is_bool || !starts_with_dash(without_default_value))
        {
          ASSERT_EQ(params_copy.get<type>("without_default"), without_default_value);
        }
        if constexpr (!is_bool)
        {
          if (!starts_with_dash(required_value))
          {
            ASSERT_EQ(params_copy.get<type>("required"), required_value);
          }
          if (!starts_with_dash(set_default_value))
          {
            ASSERT_EQ(params_copy.get<type>("with_default"), set_default_value);
          }
        }
      }
    }

    if constexpr (!is_bool)
    {
      // Test that the entry with a default value has its default value
      {
        CommandLine cl;
        cl.addArgument("/path/to/exe");
        cl.parse();

        auto params_copy = params;
        cl.populateCommandLineParams(params_copy);
        ASSERT_EQ(params_copy.get<type>("with_default"), default_value);
      }

      // Test an empty initializer list default
      {
        InputParameters empty_default_params = emptyInputParameters();
        empty_default_params.addCommandLineParam<type>(
            "empty_default", "--empty-default", {}, "Doc4");

        CommandLine cl;
        cl.addArgument("/path/to/exe");
        cl.parse();
        cl.populateCommandLineParams(empty_default_params);
        ASSERT_EQ(empty_default_params.get<type>("empty_default"), type{});
      }
    }
  };

  test_populate(std::string(),
                std::string("foo bar"),
                std::string("bar bang"),
                std::string("defaultfoo abcd"),
                std::string("setdefaultfoo"));
  test_populate(std::vector<std::string>(),
                std::vector<std::string>({"foo", "bar"}),
                std::vector<std::string>({"bar"}),
                std::vector<std::string>({"nonemptydefault"}),
                std::vector<std::string>({"hello", "cool", "default"}));
  test_populate(Real(), Real(1e-6), Real(100.1), Real(5), Real(1e-5));
  test_populate(uint(), uint(100), uint(5), uint(0), uint(1e6));
  test_populate(int(), int(-200), int(-800), int(26), int(1e6));
  test_populate(bool(), bool(true), nullptr, nullptr, nullptr);
}

TEST(CommandLineTest, populateBadInterpret)
{
  const auto test = [](const auto & value_type, const std::string & value)
  {
    using type = typename std::remove_const<
        typename std::remove_reference<decltype(value_type)>::type>::type;

    InputParameters params = emptyInputParameters();
    params.addCommandLineParam<type>("value", "--value", "Doc");

    CommandLine cl;
    cl.addArgument("/path/to/exe");
    cl.addArgument("--value=" + value);
    cl.parse();

    try
    {
      cl.populateCommandLineParams(params);
      FAIL();
    }
    catch (const std::exception & err)
    {
      ASSERT_EQ(std::string(err.what()),
                "While parsing command line option '--value' with value '" + value +
                    "':\n\nUnable to "
                    "convert '" +
                    value + "' to type " + MooseUtils::prettyCppType<type>());
    }
  };

  test(Real(), "abcd");
  test(uint(), "!456");
  test(int(), "XYZ?");
}

TEST(CommandLineTest, populateSameSwitch)
{
  InputParameters params = emptyInputParameters();
  params.addCommandLineParam<bool>("value", "--value", "Doc");
  params.addCommandLineParam<bool>("value2", "--value", "Doc");

  CommandLine cl;
  cl.parse();

  try
  {
    cl.populateCommandLineParams(params);
    FAIL();
  }
  catch (const std::exception & err)
  {
    ASSERT_EQ(std::string(err.what()),
              "The command line options 'value' and 'value2' both declare the command line switch "
              "'--value'");
  }
}

TEST(CommandLineTest, populateMooseEnum)
{
  InputParameters params = emptyInputParameters();
  const auto default_value = "foo";
  MooseEnum enum_values("foo bar", default_value);
  params.addCommandLineParam<MooseEnum>("value", "--value", enum_values, "Doc");

  const auto check = [params, &default_value](const std::string & value)
  {
    auto params_copy = params;
    CommandLine cl;
    if (value.size())
    {
      cl.addArgument("--value");
      cl.addArgument(value);
    }
    cl.parse();
    cl.populateCommandLineParams(params_copy);

    EXPECT_EQ((std::string)params_copy.get<MooseEnum>("value"),
              value.size() ? value : default_value);
  };

  check("");
  check("foo");
  check("bar");
  try
  {
    check("baz");
    FAIL();
  }
  catch (const std::exception & err)
  {
    ASSERT_EQ(std::string(err.what()),
              "While parsing command line option '--value' with value 'baz':\n\nInvalid option "
              "\"baz\" in MooseEnum.  Valid options (not case-sensitive) are \"foo bar\".");
  }
}

TEST(CommandLineTest, populateSetByUser)
{
  InputParameters params = emptyInputParameters();
  params.addCommandLineParam<bool>("value", "--value", "Doc");
  EXPECT_FALSE(params.isParamSetByUser("value"));

  {
    CommandLine cl;
    cl.addArgument("/path/to/exe");
    cl.addArgument("--value");
    cl.parse();

    auto params_copy = params;
    cl.populateCommandLineParams(params_copy);
    EXPECT_TRUE(params_copy.isParamSetByUser("value"));
  }

  {
    CommandLine cl;
    cl.addArgument("/path/to/exe");
    cl.parse();

    auto params_copy = params;
    cl.populateCommandLineParams(params_copy);
    EXPECT_FALSE(params_copy.isParamSetByUser("value"));
  }
}

TEST(CommandLineTest, initSubAppCommandLine)
{
  const auto test = [](const std::vector<std::string> & args,
                       const std::string & multiapp_name,
                       const std::string & subapp_name,
                       const std::string & subsubapp_name,
                       const std::vector<std::string> & expected_args,
                       const std::vector<std::string> & subexpected_args)
  {
    InputParameters params = emptyInputParameters();
    params.addCommandLineParam<bool>("global", "--global", "Doc1");
    params.setGlobalCommandLineParam("global");
    params.addCommandLineParam<bool>("another_global", "--another-global", "Doc2");
    params.setGlobalCommandLineParam("another_global");
    params.addCommandLineParam<bool>("not_global", "--not-global", "Doc3");

    CommandLine cl;
    cl.addArgument("/path/to/exe");
    cl.addArguments(args);
    cl.parse();
    cl.populateCommandLineParams(params);

    const auto subapp_cl =
        cl.initSubAppCommandLine(multiapp_name, subapp_name, std::vector<std::string>());
    ASSERT_EQ(expected_args, subapp_cl->getArguments());
    subapp_cl->parse();
    subapp_cl->populateCommandLineParams(params);

    const auto subsubapp_cl =
        subapp_cl->initSubAppCommandLine(multiapp_name, subsubapp_name, std::vector<std::string>());
    ASSERT_EQ(subexpected_args, subsubapp_cl->getArguments());
    subsubapp_cl->parse();
  };

  test({"--global",
        "--not-global",
        "-not-used",
        "sub:some/value=123",
        "othersub0:value=1",
        "sub0:App/foo=bar",
        ":Foo/bar=baz",
        "sub0:subsub0:val=5",
        "sub1:Kernels/active=foo"},
       "sub",
       "sub0",
       "subsub0",
       {"--global", "some/value=123", "App/foo=bar", ":Foo/bar=baz", "subsub0:val=5"},
       {"--global", ":Foo/bar=baz", "val=5"});
  test({"--unused", "--another_global"}, "sub", "sub1", "subsub1", {}, {});
}

TEST(CommandLineTest, globalCommandLineParamSubapp)
{
  InputParameters params = emptyInputParameters();
  params.addCommandLineParam<bool>("global", "--global", "Doc1");
  params.setGlobalCommandLineParam("global");
  InputParameters subapp_params = params;
  InputParameters subsubapp_params = params;

  CommandLine cl;
  cl.addArgument("/path/to/exe");
  cl.addArgument("--global");
  cl.parse();
  cl.populateCommandLineParams(params);
  ASSERT_TRUE(params.get<bool>("global"));

  const auto subapp_cl = cl.initSubAppCommandLine("sub", "sub0", {});
  subapp_cl->parse();
  subapp_cl->populateCommandLineParams(subapp_params);
  ASSERT_TRUE(subapp_params.get<bool>("global"));

  const auto subsubapp_cl = cl.initSubAppCommandLine("subsub", "subsub0", {});
  subsubapp_cl->parse();
  subsubapp_cl->populateCommandLineParams(subsubapp_params);
  ASSERT_TRUE(subsubapp_params.get<bool>("global"));
}

TEST(CommandLineTest, requiredParameter)
{
  InputParameters params = emptyInputParameters();
  params.addRequiredCommandLineParam<std::string>("value", "--value", "Doc");

  {
    auto params_copy = params;
    CommandLine cl;
    cl.addArgument("/path/to/exe");
    cl.parse();

    try
    {
      cl.populateCommandLineParams(params_copy);
      FAIL();
    }
    catch (const std::exception & err)
    {
      EXPECT_EQ((std::string)err.what(),
                "Missing required command-line parameter: value\nDoc string: Doc");
    }
  }

  {
    auto params_copy = params;
    CommandLine cl;
    cl.addArgument("/path/to/exe");
    cl.addArgument("--value=foo");
    cl.parse();
    cl.populateCommandLineParams(params_copy);
    EXPECT_EQ(params_copy.get<std::string>("value"), "foo");
  }
}

TEST(CommandLineTest, requiredParameterArgument)
{
  const auto check = [](auto & params, const bool fail)
  {
    CommandLine cl;
    cl.addArgument("/path/to/exe");
    cl.addArgument("--value");
    cl.parse();

    try
    {
      cl.populateCommandLineParams(params);
      if (fail)
        FAIL();
    }
    catch (const std::exception & err)
    {
      ASSERT_EQ(std::string(err.what()),
                "The command line option '--value' requires a value and one was not provided.\nDoc "
                "string: Doc");
    }
  };

  {
    InputParameters params = emptyInputParameters();
    params.addCommandLineParam<std::string>("value", "--value", "Doc");
    check(params, true);
  }

  {
    InputParameters params = emptyInputParameters();
    params.addCommandLineParam<std::string>("value", "--value", "some_value", "Doc");
    check(params, false);
  }

  {
    InputParameters params = emptyInputParameters();
    const auto default_value = "foo";
    MooseEnum enum_values("foo", default_value);
    params.addCommandLineParam<MooseEnum>("value", "--value", enum_values, "Doc");
    check(params, true);
  }
}

TEST(CommandLineTest, duplicateOptions)
{
  InputParameters params = emptyInputParameters();
  params.addCommandLineParam<unsigned int>("value", "-v --value", "Doc");

  CommandLine cl;
  cl.addArgument("/path/to/exe");
  cl.addArgument("-v");
  cl.addArgument("1");
  cl.addArgument("--value");
  cl.addArgument("2");
  cl.addArgument("-v");
  cl.addArgument("3");
  cl.parse();

  cl.populateCommandLineParams(params);

  ASSERT_EQ(params.get<unsigned int>("value"), uint(3));
}

TEST(CommandLineTest, unappliedArgument)
{
  const auto test_not_applied = [](const std::vector<std::string> & args,
                                   const std::string & not_applied_arg,
                                   const std::string & suggestion = "")
  {
    CommandLine cl;
    cl.addArgument("/path/to/exe");
    for (const auto & arg : args)
      cl.addArgument(arg);

    try
    {
      cl.parse();
      FAIL();
    }
    catch (const std::exception & err)
    {
      std::string expected_err = "The command line argument '" + not_applied_arg +
                                 "' is not applied to an option and is not a HIT parameter.";
      if (suggestion.size())
        expected_err +=
            "\n\nDid you mean to combine this argument with the previous argument, such as:\n\n  " +
            suggestion + "\n";
      ASSERT_EQ(std::string(err.what()), expected_err);
    }
  };

  test_not_applied({"foo"}, "foo");
  test_not_applied({"foo=bar", "coolio"}, "coolio", "foo='bar coolio'");
}

TEST(CommandLineTest, boolParamWithValue)
{
  InputParameters params = emptyInputParameters();
  params.addCommandLineParam<bool>("value", "--value", "Doc");

  CommandLine cl;
  cl.addArgument("/path/to/exe");
  cl.addArgument("--value");
  cl.addArgument("foo");
  cl.parse();

  try
  {
    cl.populateCommandLineParams(params);
    FAIL();
  }
  catch (const std::exception & err)
  {
    ASSERT_EQ(std::string(err.what()),
              "The command line option '--value' is a boolean and does not support a value but the "
              "value 'foo' was provided.\nDoc string: Doc");
  }
}

TEST(CommandLineTest, negativeScalarParam)
{
  const auto test = [](auto value)
  {
    using type = typename std::remove_reference<decltype(value)>::type;

    {
      CommandLine cl;
      cl.addArgument("/path/to/exe");
      cl.addArgument("--value");
      cl.addArgument(std::to_string(value));
      cl.parse();

      InputParameters params = emptyInputParameters();
      params.addCommandLineParam<type>("value", "--value", "Doc");
      cl.populateCommandLineParams(params);

      ASSERT_EQ(params.get<type>("value"), value);
    }

    {
      CommandLine cl;
      cl.addArgument("/path/to/exe");
      cl.addArgument("--value");
      cl.addArgument(std::to_string(value));
      cl.parse();

      InputParameters params = emptyInputParameters();
      params.addCommandLineParam<type>("value", "--value", value - 100, "Doc");
      cl.populateCommandLineParams(params);

      ASSERT_EQ(params.get<type>("value"), value);
    }
  };

  test(Real(-1.2));
  test(int(-2));
}

TEST(CommandLineTest, mergeArgsForParam)
{
  const auto test = [](const std::string & argument)
  {
    {
      CommandLine cl;
      cl.addArgument("/path/to/exe");
      cl.addArgument("--value");
      cl.addArgument(argument);
      cl.parse();

      InputParameters params = emptyInputParameters();
      params.addCommandLineParam<std::vector<std::string>>("value", "--value", "Doc");
      cl.populateCommandLineParams(params);

      std::vector<std::string> split_argument;
      MooseUtils::tokenize(argument, split_argument, 1, " \t\n\v\f\r");
      ASSERT_EQ(params.get<std::vector<std::string>>("value"), split_argument);
    }

    {
      CommandLine cl;
      cl.addArgument("/path/to/exe");
      cl.addArgument("--value");
      cl.addArgument(argument);
      cl.parse();

      InputParameters params = emptyInputParameters();
      params.addCommandLineParam<std::string>("value", "--value", "Doc");
      cl.populateCommandLineParams(params);

      ASSERT_EQ(params.get<std::string>("value"), argument);
    }
  };

  test("petsc>=3.2 & slepc<=1.0");
  test("petsc & slepc<=1.0");
  test("slepc & petsc=1");
  test("petsc==1.5 & slepc");
  test("!neml & !petsc>=1.0");
  test("neml    & foo=bar");
  test("petsc>=3.11 & ad_size=30 & libtorch");
  test("(neml) & !(petsc<3 & !libtorch)");
}

TEST(CommandLineTest, findCommandLineParam)
{
  CommandLine cl;
  cl.addArgument("/path/to/exe");
  cl.addArgument("--value");
  cl.addArgument("foo");
  cl.parse();

  InputParameters params = emptyInputParameters();
  params.addCommandLineParam<std::string>("value", "--value", "Doc");
  params.addCommandLineParam<std::string>("other_value", "--other_value", "Doc");
  cl.populateCommandLineParams(params);

  auto find_value = std::as_const(cl).findCommandLineParam("value");
  auto value_entry = std::next(std::as_const(cl).getEntries().begin());
  ASSERT_EQ(find_value, value_entry);

  auto find_other_value = std::as_const(cl).findCommandLineParam("other_value");
  ASSERT_EQ(find_other_value, std::as_const(cl).getEntries().end());

  try
  {
    std::as_const(cl).findCommandLineParam("foo");
    FAIL();
  }
  catch (const std::exception & err)
  {
    ASSERT_EQ(
        std::string(err.what()),
        "CommandLine::findCommandLineParam(): The parameter 'foo' is not a command line parameter");
  }
}

TEST(CommandLineTest, disallowApplicationType)
{
  {
    CommandLine cl;
    cl.addArgument("/path/to/exe");
    cl.addArgument("Application/type=Foo");

    try
    {
      cl.parse();
    }
    catch (const std::exception & err)
    {
      ASSERT_EQ(std::string(err.what()),
                "The command line argument 'Application/type=Foo' is not allowed.\nThe application "
                "type must be set via the --app command line option.");
    }
  }

  {
    CommandLine cl;
    cl.addArgument("/path/to/exe");
    cl.addArgument("sub0:Application/type=Foo");

    try
    {
      cl.parse();
    }
    catch (const std::exception & err)
    {
      ASSERT_EQ(std::string(err.what()),
                "The command line argument 'sub0:Application/type=Foo' is not allowed.\nThe "
                "application type must be set via the MultiApp type input parameter or the "
                "Application/type parameter in the MultiApp input.");
    }
  }
}

TEST(CommandLineTest, optionalValuedParam)
{
  {
    // Because --mesh-only is optional, we shouldn't associate
    // the value foo=bar with it because it could be seen as
    // a hit parameter (this preserves old behavior)
    CommandLine cl;
    cl.addArgument("/path/to/exe");
    cl.addArgument("--mesh-only");
    cl.addArgument("foo=bar");
    cl.parse();

    InputParameters params = emptyInputParameters();
    params.addOptionalValuedCommandLineParam<std::string>("mesh_only", "--mesh-only", {}, "Doc");
    cl.populateCommandLineParams(params);

    ASSERT_TRUE(params.isParamSetByUser("mesh_only"));
    ASSERT_TRUE(params.get<std::string>("mesh_only").empty());
    ASSERT_EQ(cl.buildHitParams(), "foo=bar");
  }

  {
    // We should associate "foo" with --mesh-only (optional)
    // because it is not a hit param
    CommandLine cl;
    cl.addArgument("/path/to/exe");
    cl.addArgument("--mesh-only");
    cl.addArgument("foo");
    cl.parse();

    InputParameters params = emptyInputParameters();
    params.addOptionalValuedCommandLineParam<std::string>("mesh_only", "--mesh-only", {}, "Doc");
    cl.populateCommandLineParams(params);

    ASSERT_EQ(params.get<std::string>("mesh_only"), std::string("foo"));
  }
}

TEST(CommandLineTest, mergeHIT)
{
  CommandLine cl;
  cl.addArgument("/path/to/exe");
  cl.addArgument("Foo/bar=baz");
  cl.addArgument("Foo/bar=bang");
  cl.parse();

  InputParameters params = emptyInputParameters();
  cl.populateCommandLineParams(params);

  ASSERT_EQ(cl.buildHitParams(), "Foo/bar=baz Foo/bar=bang");
}

TEST(CommandLineTest, removeArgument)
{
  CommandLine cl;
  cl.addArgument("/path/to/exe");
  cl.addArgument("--arg");
  EXPECT_TRUE(cl.hasArgument("--arg"));
  cl.removeArgument("--arg");
  EXPECT_FALSE(cl.hasArgument("--arg"));
}

TEST(CommandLineTest, removeArgumentMissing)
{
  CommandLine cl;
  cl.addArgument("/path/to/exe");

  EXPECT_FALSE(cl.hasArgument("--arg"));
  try
  {
    cl.removeArgument("--arg");
    FAIL();
  }
  catch (const std::exception & e)
  {
    EXPECT_EQ(std::string(e.what()),
              "CommandLine::removeArgument(): The argument '--arg' does not exist");
  }
}
