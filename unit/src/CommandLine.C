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

TEST(CommandLine, parse)
{
  {
    InputParameters params = emptyInputParameters();

    params.addCommandLineParam<bool>(
        "bool_param_with_default", "--bool_param_with_default", true, "Doc");
    params.addCommandLineParam<int>(
        "int_param_with_default", "--int_param_with_default", 42, "Doc");
    params.addCommandLineParam<std::string>(
        "string_param_with_default", "--string_param_with_default", "stuff", "Doc");

    params.addCommandLineParam<bool>(
        "bool_param_without_default", "--bool_param_without_default", "Doc");
    params.addCommandLineParam<int>(
        "int_param_without_default", "--int_param_without_default", "Doc");
    params.addCommandLineParam<std::string>(
        "string_param_without_default", "--string_param_without_default", "Doc");

    params.addCommandLineParam<std::string>(
        "compound_string_param", "--compound_string_param", "Doc");

    params.addCommandLineParam<std::string>(
        "compound_string_param_with_equals", "--compound_string_param_with_equals", "Doc");

    int argc = 9;
    const char * argv[9] = {"--bool_param_without_default",
                            "true",
                            "--int_param_without_default",
                            "43",
                            "--string_param_without_default",
                            "astring",
                            "--compound_string_param",
                            "astring anotherstring",

                            // Note that Bash will strip quotes automatically.
                            // So this next one is really for
                            // --compound_string_param_with_equals='astring anotherstring'
                            "--compound_string_param_with_equals=astring anotherstring"};

    CommandLine cl(argc, (char **)argv);

    cl.addCommandLineOptionsFromParams(params);

    cl.populateInputParams(params);

    EXPECT_EQ(params.get<bool>("bool_param_with_default"), true);
    EXPECT_EQ(params.get<int>("int_param_with_default"), 42);
    EXPECT_EQ(params.get<std::string>("string_param_with_default"), "stuff");

    EXPECT_EQ(params.get<bool>("bool_param_without_default"), true);
    EXPECT_EQ(params.get<int>("int_param_without_default"), 43);
    EXPECT_EQ(params.get<std::string>("string_param_without_default"), "astring");

    EXPECT_EQ(params.get<std::string>("compound_string_param"), "astring anotherstring");
    EXPECT_EQ(params.get<std::string>("compound_string_param_with_equals"),
              "astring anotherstring");
  }
}

TEST(CommandLine, initForMultiApp)
{
  int argc = 6;
  const char * argv[6] = {
      "--flag", "sub0:year=2011", "sub1:year=2013", "sub:month=1", "notsub:day=2", ":niner=9"};
  {
    CommandLine cl(argc, (char **)argv);

    std::vector<std::string> gold = {
        "--flag", "sub0:year=2011", "sub1:year=2013", "sub:month=1", "notsub:day=2", ":niner=9"};
    EXPECT_EQ(cl.getArguments(), gold);

    cl.initForMultiApp("sub0");
    gold = {"--flag", "sub0:year=2011", "sub:month=1", ":niner=9"};
    EXPECT_EQ(cl.getArguments(), gold);
  }

  {
    CommandLine cl(argc, (char **)argv);
    cl.initForMultiApp("sub1");
    std::vector<std::string> gold = {"--flag", "sub1:year=2013", "sub:month=1", ":niner=9"};
    EXPECT_EQ(cl.getArguments(), gold);
  }

  {
    CommandLine cl(argc, (char **)argv);
    cl.initForMultiApp("notsub0");
    std::vector<std::string> gold = {"--flag", "notsub:day=2", ":niner=9"};
    EXPECT_EQ(cl.getArguments(), gold);
  }
}

TEST(CommandLine, requiredArgument)
{
  InputParameters params = emptyInputParameters();
  const auto default_value = "foo";
  MooseEnum enum_values("foo", default_value);
  params.addCommandLineParam<MooseEnum>("value", "--value", enum_values, "Doc");

  CommandLine cl;
  cl.addArgument("--value");
  cl.addCommandLineOptionsFromParams(params);

  try
  {
    cl.populateInputParams(params);
    FAIL();
  }
  catch (const std::exception & err)
  {
    const auto pos =
        std::string(err.what())
            .find("The command line argument '--value' requires a value and one was not provided");
    ASSERT_TRUE(pos != std::string::npos);
  }
}

TEST(CommandLine, setMooseEnum)
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
    cl.addCommandLineOptionsFromParams(params_copy);
    cl.populateInputParams(params_copy);

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
    const std::string expected_err = "While parsing command line argument '--value' with value "
                                     "'baz':\n\nInvalid option \"baz\" in MooseEnum.";
    const auto pos = std::string(err.what()).find(expected_err);
    EXPECT_TRUE(pos != std::string::npos);
  }
}
