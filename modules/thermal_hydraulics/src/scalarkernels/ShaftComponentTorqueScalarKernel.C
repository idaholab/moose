//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ShaftComponentTorqueScalarKernel.h"
#include "MooseVariableScalar.h"
#include "Assembly.h"
#include "UserObject.h"
#include "ShaftConnectableUserObjectInterface.h"

registerMooseObject("ThermalHydraulicsApp", ShaftComponentTorqueScalarKernel);

InputParameters
ShaftComponentTorqueScalarKernel::validParams()
{
  InputParameters params = ScalarKernel::validParams();
  params.addRequiredParam<UserObjectName>("shaft_connected_component_uo",
                                          "Shaft connected component user object name");
  params.addClassDescription("Torque contributed by a component connected to a shaft");
  return params;
}

ShaftComponentTorqueScalarKernel::ShaftComponentTorqueScalarKernel(
    const InputParameters & parameters)
  : ScalarKernel(parameters),
    _shaft_connected_component_uo(
        getUserObject<ShaftConnectableUserObjectInterface>("shaft_connected_component_uo"))
{
}

void
ShaftComponentTorqueScalarKernel::reinit()
{
}

void
ShaftComponentTorqueScalarKernel::computeResidual()
{
  prepareVectorTag(_assembly, _var.number());

  _local_re(0) -= _shaft_connected_component_uo.getTorque();

  accumulateTaggedLocalResidual();
}

void
ShaftComponentTorqueScalarKernel::computeJacobian()
{
  DenseMatrix<Real> jacobian_block;
  std::vector<dof_id_type> dofs_j;
  _shaft_connected_component_uo.getTorqueJacobianData(jacobian_block, dofs_j);
  jacobian_block.scale(-1);
  addJacobian(_assembly, jacobian_block, _var.dofIndices(), dofs_j, _var.scalingFactor());
}
