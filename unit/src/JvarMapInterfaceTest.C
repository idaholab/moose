//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "gtest_include.h"

#include "Executioner.h"
#include "FEProblemBase.h"
#include "JvarMapInterface.h"
#include "Kernel.h"
#include "MooseAppTestUtils.h"
#include "MooseMain.h"
#include "NonlinearSystemBase.h"

#include <string>

using MooseAppTestUtils::Args;

/**
 * Kernel with a zero residual that exposes the JvarMapInterface maps for inspection
 */
class JvarMapInterfaceTestKernel : public JvarMapKernelInterface<Kernel>
{
public:
  static InputParameters validParams()
  {
    auto params = JvarMapKernelInterface<Kernel>::validParams();
    params.addCoupledVar("v0", "Coupled variables for the parameter-specific map");
    return params;
  }

  JvarMapInterfaceTestKernel(const InputParameters & parameters)
    : JvarMapKernelInterface<Kernel>(parameters), _v0_map(getParameterJvarMap("v0"))
  {
  }

  /// Map built by getParameterJvarMap for the "v0" parameter
  const JvarMap & _v0_map;

protected:
  virtual Real computeQpResidual() override { return 0; }
};

registerMooseObject("MooseUnitApp", JvarMapInterfaceTestKernel);

// The kernel lives in a system that holds u (number 0) and v (number 1). It also couples w, number
// 0 in a second nonlinear system, and a, number 0 in the auxiliary system. Variable numbers are
// local to each system, so only u may be mapped to the number-0 slot; w and a have no Jacobian
// columns in the kernel's system and must not appear in either map.
TEST(JvarMapInterface, SkipsVariablesFromOtherSystems)
{
  Args args({"-i", "files/JvarMapInterfaceTest/multi_system.i"});
  const auto app = Moose::createMooseApp("MooseUnitApp", args.argc(), args.argv());
  app->run();
  auto & fe_problem = app->getExecutioner()->feProblem();

  auto & sys = fe_problem.getNonlinearSystemBase(fe_problem.nlSysNum("uv_sys"));
  const auto kernel = std::dynamic_pointer_cast<JvarMapInterfaceTestKernel>(
      sys.getKernelWarehouse().getObject("test"));
  ASSERT_TRUE(kernel);

  const auto u_number = sys.getVariable(0, "u").number();
  const auto v_number = sys.getVariable(0, "v").number();

  // map over all coupled variables: one entry per variable of the kernel's system
  const auto & map = kernel->getJvarMap();
  ASSERT_EQ(map.size(), sys.nVariables());
  ASSERT_GE(map[u_number], 0);
  EXPECT_EQ(kernel->getCoupledMooseVars()[map[u_number]]->name(), "u");
  // the kernel variable is always marked as coupled
  EXPECT_EQ(map[v_number], 0);

  // map for the "v0" parameter, which indexes into the parameter's own variable list
  ASSERT_EQ(kernel->_v0_map.size(), sys.nVariables());
  EXPECT_EQ(kernel->_v0_map[u_number], 0);
  EXPECT_EQ(kernel->_v0_map[v_number], -1);
}
