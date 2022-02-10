//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "VolumeJunctionAdvectionScalarKernel.h"
#include "MooseVariableScalar.h"
#include "VolumeJunctionBaseUserObject.h"

registerMooseObject("ThermalHydraulicsApp", VolumeJunctionAdvectionScalarKernel);

InputParameters
VolumeJunctionAdvectionScalarKernel::validParams()
{
  InputParameters params = ScalarKernel::validParams();

  params.addRequiredParam<unsigned int>("equation_index", "Equation index");
  params.addRequiredParam<UserObjectName>("volume_junction_uo", "Volume junction user object name");

  params.addClassDescription(
      "Adds advective fluxes for the junction variables for a volume junction");

  return params;
}

VolumeJunctionAdvectionScalarKernel::VolumeJunctionAdvectionScalarKernel(
    const InputParameters & params)
  : ScalarKernel(params),

    _equation_index(getParam<unsigned int>("equation_index")),
    _volume_junction_uo(getUserObject<VolumeJunctionBaseUserObject>("volume_junction_uo"))
{
  if (_var.order() > 1)
    mooseError(name(), ": This scalar kernel can be used only with first-order scalar variables.");
}

void
VolumeJunctionAdvectionScalarKernel::reinit()
{
}

void
VolumeJunctionAdvectionScalarKernel::computeResidual()
{
  prepareVectorTag(_assembly, _var.number());

  for (_i = 0; _i < _var.order(); _i++)
    _local_re(_i) += _volume_junction_uo.getResidual()[_equation_index];

  accumulateTaggedLocalResidual();
}

void
VolumeJunctionAdvectionScalarKernel::computeJacobian()
{
  DenseMatrix<Real> jacobian_block;
  std::vector<dof_id_type> dofs_i, dofs_j;
  _volume_junction_uo.getScalarEquationJacobianData(
      _equation_index, jacobian_block, dofs_i, dofs_j);

  _assembly.cacheJacobianBlock(jacobian_block, dofs_i, dofs_j, _var.scalingFactor());
}
