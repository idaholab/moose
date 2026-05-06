//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "gtest/gtest.h"

#include "MooseObjectUnitTest.h"
#include "MooseVariableFE.h"
#include "MooseMesh.h"

/**
 * Fixture: mesh + FEProblem with a scalar and a 2-component array aux variable.
 * DOFs are numbered and the semilocal node list is populated so reinitNodes works.
 */
class MooseVariableDataReinitNodesTest : public MooseObjectUnitTest
{
public:
  MooseVariableDataReinitNodesTest() : MooseObjectUnitTest("MooseUnitApp")
  {
    {
      InputParameters params = _factory.getValidParams("MooseVariable");
      params.set<MooseEnum>("order") = "FIRST";
      params.set<MooseEnum>("family") = "LAGRANGE";
      _fe_problem->addAuxVariable("MooseVariable", "scalar_var", params);
    }
    {
      InputParameters params = _factory.getValidParams("ArrayMooseVariable");
      params.set<MooseEnum>("order") = "FIRST";
      params.set<MooseEnum>("family") = "LAGRANGE";
      params.set<unsigned int>("components") = 2;
      _fe_problem->addAuxVariable("ArrayMooseVariable", "array_var", params);
    }

    _fe_problem->es().init();
    // MooseVariableData checks isSemiLocal so we have to populate this range
    std::set<dof_id_type> no_ghosts;
    _mesh->updateActiveSemiLocalNodeRange(no_ghosts);

    for (const auto & node : _mesh->getMesh().local_node_ptr_range())
      _node_ids.push_back(node->id());
  }

protected:
  std::vector<dof_id_type> _node_ids;
};

TEST_F(MooseVariableDataReinitNodesTest, scalarVariableDofsMatchLibmesh)
{
  auto & var = _fe_problem->getStandardVariable(0, "scalar_var");
  var.reinitNodes(_node_ids);

  const auto sys_num = var.sys().number();
  const auto var_num = var.number();

  std::vector<dof_id_type> expected;
  for (const auto node_id : _node_ids)
  {
    const auto & nd = _mesh->getMesh().node_ref(node_id);
    expected.push_back(nd.dof_number(sys_num, var_num, 0));
  }

  EXPECT_EQ(var.dofIndices(), expected);
}

TEST_F(MooseVariableDataReinitNodesTest, arrayVariableDofsMatchLibmesh)
{
  auto & var = _fe_problem->getArrayVariable(0, "array_var");
  var.reinitNodes(_node_ids);

  const auto & dof_map = var.dofMap();
  const auto var_num = var.number();

  std::vector<dof_id_type> expected;
  std::vector<dof_id_type> node_component_dofs;
  for (const auto node_id : _node_ids)
  {
    const auto & nd = _mesh->getMesh().node_ref(node_id);
    dof_map.array_dof_indices(&nd, node_component_dofs, var_num);
    expected.insert(expected.end(), node_component_dofs.begin(), node_component_dofs.end());
  }

  EXPECT_EQ(var.dofIndices(), expected);
}
