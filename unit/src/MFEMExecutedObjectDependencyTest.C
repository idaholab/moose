//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMObjectUnitTest.h"
#include "MFEMAuxKernel.h"
#include "MFEMPostprocessor.h"

#include <string>
#include <vector>

namespace
{
std::vector<std::string> execution_log;
}

class TestMFEMDependencyAux : public MFEMAuxKernel
{
public:
  static InputParameters validParams();

  TestMFEMDependencyAux(const InputParameters & parameters);

  virtual void execute() override;

private:
  const bool _use_postprocessor;
};

registerMooseObject("MooseUnitApp", TestMFEMDependencyAux);

InputParameters
TestMFEMDependencyAux::validParams()
{
  auto params = MFEMAuxKernel::validParams();
  params.addParam<PostprocessorName>(
      "postprocessor", "Optional postprocessor dependency used to set the output value.");
  return params;
}

TestMFEMDependencyAux::TestMFEMDependencyAux(const InputParameters & parameters)
  : MFEMAuxKernel(parameters), _use_postprocessor(parameters.isParamSetByUser("postprocessor"))
{
}

void
TestMFEMDependencyAux::execute()
{
  execution_log.push_back(name());

  const auto value = _use_postprocessor ? getPostprocessorValue("postprocessor") + 1.0 : 1.0;
  mfem::ConstantCoefficient coef(value);
  _result_var.ProjectCoefficient(coef);
}

class TestMFEMDependencyPostprocessor : public MFEMPostprocessor
{
public:
  static InputParameters validParams();

  TestMFEMDependencyPostprocessor(const InputParameters & parameters);

  virtual void execute() override;
  virtual PostprocessorValue getValue() const override;

private:
  const VariableName _variable_name;
  mfem::ParGridFunction & _variable;
  PostprocessorValue _value = 0.0;
};

registerMooseObject("MooseUnitApp", TestMFEMDependencyPostprocessor);

InputParameters
TestMFEMDependencyPostprocessor::validParams()
{
  auto params = MFEMPostprocessor::validParams();
  params.addRequiredParam<VariableName>("variable",
                                        "The MFEM variable this test postprocessor reads.");
  return params;
}

TestMFEMDependencyPostprocessor::TestMFEMDependencyPostprocessor(const InputParameters & parameters)
  : MFEMPostprocessor(parameters),
    _variable_name(getParam<VariableName>("variable")),
    _variable(*getMFEMProblem().getProblemData().gridfunctions.Get(_variable_name))
{
}

void
TestMFEMDependencyPostprocessor::execute()
{
  execution_log.push_back(name());
  _value = _variable.GetData()[0];
}

PostprocessorValue
TestMFEMDependencyPostprocessor::getValue() const
{
  return _value;
}

class MFEMExecutedObjectDependencyTest : public MFEMObjectUnitTest
{
public:
  MFEMExecutedObjectDependencyTest() : MFEMObjectUnitTest("MooseUnitApp")
  {
    InputParameters fe_params = _factory.getValidParams("MFEMScalarFESpace");
    fe_params.set<MooseEnum>("fec_type") = "H1";
    _mfem_problem->addFESpace("MFEMScalarFESpace", "h1", fe_params);

    InputParameters variable_params = _factory.getValidParams("MFEMVariable");
    variable_params.set<UserObjectName>("fespace") = "h1";
    _mfem_problem->addVariable("MFEMVariable", "aux0_var", variable_params);
    _mfem_problem->addVariable("MFEMVariable", "aux1_var", variable_params);
  }

  virtual void SetUp() override { execution_log.clear(); }
};

TEST_F(MFEMExecutedObjectDependencyTest, PostprocessorAuxKernelDependencyChain)
{
  InputParameters aux0_params = _factory.getValidParams("TestMFEMDependencyAux");
  aux0_params.set<AuxVariableName>("variable") = "aux0_var";
  addObject<TestMFEMDependencyAux>("TestMFEMDependencyAux", "aux0", aux0_params);

  InputParameters pp0_params = _factory.getValidParams("TestMFEMDependencyPostprocessor");
  pp0_params.set<VariableName>("variable") = "aux0_var";
  auto & pp0 = addObject<TestMFEMDependencyPostprocessor>(
      "TestMFEMDependencyPostprocessor", "pp0", pp0_params);

  InputParameters aux1_params = _factory.getValidParams("TestMFEMDependencyAux");
  aux1_params.set<AuxVariableName>("variable") = "aux1_var";
  aux1_params.set<PostprocessorName>("postprocessor") = "pp0";
  addObject<TestMFEMDependencyAux>("TestMFEMDependencyAux", "aux1", aux1_params);

  InputParameters pp1_params = _factory.getValidParams("TestMFEMDependencyPostprocessor");
  pp1_params.set<VariableName>("variable") = "aux1_var";
  auto & pp1 = addObject<TestMFEMDependencyPostprocessor>(
      "TestMFEMDependencyPostprocessor", "pp1", pp1_params);

  _mfem_problem->executeMFEMObjects(EXEC_TIMESTEP_END);

  EXPECT_EQ(execution_log, std::vector<std::string>({"aux0", "pp0", "aux1", "pp1"}));
  EXPECT_EQ(pp0.getValue(), 1.0);
  EXPECT_EQ(pp1.getValue(), 2.0);
  EXPECT_EQ(pp0.getCurrentValue(), 1.0);
  EXPECT_EQ(pp1.getCurrentValue(), 2.0);
}

#endif
