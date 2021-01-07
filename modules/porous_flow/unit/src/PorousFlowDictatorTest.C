//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PorousFlowDictatorTest.h"

TEST_F(PorousFlowDictatorTest, numVariables)
{
  EXPECT_EQ(_dictator->numVariables(), (unsigned int)3);
  EXPECT_EQ(_dictator_no_fetype->numVariables(), (unsigned int)2);
}

TEST_F(PorousFlowDictatorTest, numPhases)
{
  EXPECT_EQ(_dictator->numPhases(), (unsigned int)2);
  EXPECT_EQ(_dictator_no_fetype->numPhases(), (unsigned int)1);
}

TEST_F(PorousFlowDictatorTest, numComponents)
{
  EXPECT_EQ(_dictator->numComponents(), (unsigned int)4);
  EXPECT_EQ(_dictator_no_fetype->numComponents(), (unsigned int)3);
}

TEST_F(PorousFlowDictatorTest, numAqueousEquilibrium)
{
  EXPECT_EQ(_dictator->numAqueousEquilibrium(), (unsigned int)5);
  EXPECT_EQ(_dictator_no_fetype->numAqueousEquilibrium(), (unsigned int)0);
}

TEST_F(PorousFlowDictatorTest, numAqueousKinetic)
{
  EXPECT_EQ(_dictator->numAqueousKinetic(), (unsigned int)6);
  EXPECT_EQ(_dictator_no_fetype->numAqueousKinetic(), (unsigned int)0);
}

TEST_F(PorousFlowDictatorTest, aqueousPhaseNumber)
{
  EXPECT_EQ(_dictator->aqueousPhaseNumber(), (unsigned int)1);
  EXPECT_EQ(_dictator_no_fetype->aqueousPhaseNumber(), (unsigned int)0);
}

TEST_F(PorousFlowDictatorTest, porousFlowVariableNum)
{
  EXPECT_EQ(_dictator->porousFlowVariableNum(1), (unsigned int)0);
  EXPECT_EQ(_dictator->porousFlowVariableNum(4), (unsigned int)1);
  EXPECT_EQ(_dictator->porousFlowVariableNum(3), (unsigned int)2);

  try
  {
    _dictator->porousFlowVariableNum(0);
    FAIL();
  }
  catch (const std::exception & err)
  {
    std::size_t pos = std::string(err.what())
                          .find("The Dictator proclaims that the moose variable with number 0 is "
                                "not a PorousFlow variable.  Exiting with error code 1984.");
    ASSERT_TRUE(pos != std::string::npos);
  }

  try
  {
    _dictator->porousFlowVariableNum(2);
    FAIL();
  }
  catch (const std::exception & err)
  {
    std::size_t pos = std::string(err.what())
                          .find("The Dictator proclaims that the moose variable with number 2 is "
                                "not a PorousFlow variable.  Exiting with error code 1984.");
    ASSERT_TRUE(pos != std::string::npos);
  }

  try
  {
    _dictator->porousFlowVariableNum(5);
    FAIL();
  }
  catch (const std::exception & err)
  {
    std::size_t pos = std::string(err.what())
                          .find("The Dictator proclaims that the moose variable with number 5 is "
                                "not a PorousFlow variable.  Exiting with error code 1984.");
    ASSERT_TRUE(pos != std::string::npos);
  }

  try
  {
    _dictator->porousFlowVariableNum(123);
    FAIL();
  }
  catch (const std::exception & err)
  {
    std::size_t pos = std::string(err.what())
                          .find("The Dictator proclaims that the moose variable with number 123 is "
                                "not a PorousFlow variable.  Exiting with error code 1984.");
    ASSERT_TRUE(pos != std::string::npos);
  }

  EXPECT_EQ(_dictator_no_fetype->porousFlowVariableNum(1), (unsigned int)0);
  EXPECT_EQ(_dictator_no_fetype->porousFlowVariableNum(6), (unsigned int)1);
}

TEST_F(PorousFlowDictatorTest, mooseVariableNum)
{
  EXPECT_EQ(_dictator->mooseVariableNum(0), (unsigned int)1);
  EXPECT_EQ(_dictator->mooseVariableNum(1), (unsigned int)4);
  EXPECT_EQ(_dictator->mooseVariableNum(2), (unsigned int)3);

  try
  {
    _dictator->mooseVariableNum(3);
    FAIL();
  }
  catch (const std::exception & err)
  {
    std::size_t pos = std::string(err.what())
                          .find("The Dictator proclaims that there is no such PorousFlow variable "
                                "with number 3.  Exiting with error code 1984.");
    ASSERT_TRUE(pos != std::string::npos);
  }

  EXPECT_EQ(_dictator_no_fetype->mooseVariableNum(0), (unsigned int)1);
  EXPECT_EQ(_dictator_no_fetype->mooseVariableNum(1), (unsigned int)6);
}

TEST_F(PorousFlowDictatorTest, isPorousFlowVariable)
{
  ASSERT_TRUE(!_dictator->isPorousFlowVariable(0));
  ASSERT_TRUE(_dictator->isPorousFlowVariable(1));
  ASSERT_TRUE(!_dictator->isPorousFlowVariable(2));
  ASSERT_TRUE(_dictator->isPorousFlowVariable(3));
  ASSERT_TRUE(_dictator->isPorousFlowVariable(4));
  ASSERT_TRUE(!_dictator->isPorousFlowVariable(5));
  ASSERT_TRUE(!_dictator->isPorousFlowVariable(123));
}

TEST_F(PorousFlowDictatorTest, notPorousFlowVariable)
{
  ASSERT_TRUE(_dictator->notPorousFlowVariable(0));
  ASSERT_TRUE(!_dictator->notPorousFlowVariable(1));
  ASSERT_TRUE(_dictator->notPorousFlowVariable(2));
  ASSERT_TRUE(!_dictator->notPorousFlowVariable(3));
  ASSERT_TRUE(!_dictator->notPorousFlowVariable(4));
  ASSERT_TRUE(_dictator->notPorousFlowVariable(5));
  ASSERT_TRUE(_dictator->notPorousFlowVariable(123));
}

TEST_F(PorousFlowDictatorTest, consistentFEType)
{
  ASSERT_TRUE(_dictator->consistentFEType());
  ASSERT_TRUE(!_dictator_no_fetype->consistentFEType());
}

TEST_F(PorousFlowDictatorTest, feType)
{
  auto linear_lagrange = FEType(Utility::string_to_enum<Order>("FIRST"),
                                Utility::string_to_enum<FEFamily>("LAGRANGE"));

  ASSERT_EQ(_dictator->feType(), linear_lagrange);
}

TEST_F(PorousFlowDictatorTest, coupleAux)
{
  try
  {
    InputParameters params = _factory.getValidParams("PorousFlowDictator");
    params.set<std::vector<VariableName>>("porous_flow_vars") =
        std::vector<VariableName>{"var3", "aux_var", "var0", "var5"};
    params.set<unsigned>("number_fluid_phases") = 1;
    params.set<unsigned>("number_fluid_components") = 2;
    _fe_problem->addUserObject("PorousFlowDictator", "dictator_with_aux", params);
    FAIL();
  }
  catch (const std::exception & err)
  {
    std::size_t pos =
        std::string(err.what())
            .find(
                "PorousFlowDictator: AuxVariables variables must not be coupled into the Dictator "
                "for this is against specification #1984.  Variable number 1 is an AuxVariable.");
    ASSERT_TRUE(pos != std::string::npos);
  }
}

TEST_F(PorousFlowDictatorTest, largeAqueousPhase)
{
  try
  {
    InputParameters params = _factory.getValidParams("PorousFlowDictator");
    params.set<std::vector<VariableName>>("porous_flow_vars") = std::vector<VariableName>{"var0"};
    params.set<unsigned>("number_fluid_phases") = 1;
    params.set<unsigned>("number_fluid_components") = 2;
    params.set<unsigned>("aqueous_phase_number") = 1;
    _fe_problem->addUserObject(
        "PorousFlowDictator", "dictator_with_large_aqueous_phase_number", params);
    FAIL();
  }
  catch (const std::exception & err)
  {
    std::size_t pos =
        std::string(err.what())
            .find("PorousflowDictator: The aqueous phase number must be less than the number of "
                  "fluid phases.  The Dictator does not appreciate jokes.");
    ASSERT_TRUE(pos != std::string::npos);
  }
}
