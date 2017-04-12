/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

// MOOSE includes
#include "InputParameters.h"
#include "gtest/gtest.h"

TEST(InputParameters, checkControlParamPrivateError)
{
  try
  {
    InputParameters params = emptyInputParameters();
    params.addPrivateParam<Real>("private", 1);
    params.declareControllable("private");
    params.checkParams("");
    FAIL() << "checkParams failed to catch private control param";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_TRUE(msg.find("is a private param") != std::string::npos)
        << "failed with unexpected error: " << msg;
  }
}

// This tests for the bug https://github.com/idaholab/moose/issues/8586.
// It makes sure that range-checked input file parameters comparison functions
// do absolute floating point comparisons instead of using a default epsilon.
TEST(InputParameters, checkRangeCheckedParam)
{
  try
  {
    InputParameters params = emptyInputParameters();
    params.addRangeCheckedParam<Real>("p", 1.000000000000001, "p = 1", "Some doc");
    params.checkParams("");
    FAIL() << "range checked input param failed to catch 1.000000000000001 != 1";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_TRUE(msg.find("Range check failed for param") != std::string::npos)
        << "range check failed with unexpected error: " << msg;
  }
}

TEST(InputParameters, checkControlParamTypeError)
{
  try
  {
    InputParameters params = emptyInputParameters();
    params.addParam<PostprocessorName>("pp_name", "make_it_valid", "Some doc");
    params.declareControllable("pp_name");
    params.checkParams("");
    FAIL() << "checkParams failed to catch invalid control param type";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_TRUE(msg.find("cannot be marked as controllable because its type") != std::string::npos)
        << "failed with unexpected error:" << msg;
  }
}

TEST(InputParameters, checkControlParamValidError)
{
  try
  {
    InputParameters params = emptyInputParameters();
    params.declareControllable("not_valid");
    params.checkParams("");
    FAIL() << "checkParams failed to catch invalid control param";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_TRUE(msg.find("The parameter 'not_valid'") != std::string::npos)
        << "failed with unexpected error: " << msg;
  }
}

TEST(InputParameters, checkSuppressedError)
{
  try
  {
    InputParameters params = emptyInputParameters();
    params.suppressParameter<int>("nonexistent");
    FAIL() << "failed to error on supression of nonexisting parameter";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_TRUE(msg.find("Unable to suppress nonexistent parameter") != std::string::npos)
        << "failed with unexpected error: " << msg;
  }
}

TEST(InputParameters, checkSetDocString)
{
  InputParameters params = emptyInputParameters();
  params.addParam<Real>("little_guy", "What about that little guy?");
  params.setDocString("little_guy", "That little guy, I wouldn't worry about that little_guy.");
  ASSERT_TRUE(params.getDocString("little_guy")
                  .find("That little guy, I wouldn't worry about that little_guy.") !=
              std::string::npos)
      << "retrieved doc string has unexpected value '" << params.getDocString("little_guy") << "'";
}

TEST(InputParameters, checkSetDocStringError)
{
  try
  {
    InputParameters params = emptyInputParameters();
    params.setDocString("little_guy", "That little guy, I wouldn't worry about that little_guy.");
    FAIL() << "failed to error on attempt to set non-existing parameter";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_TRUE(msg.find("Unable to set the documentation string (using setDocString)") !=
                std::string::npos)
        << "failed with unexpected error: " << msg;
  }
}
