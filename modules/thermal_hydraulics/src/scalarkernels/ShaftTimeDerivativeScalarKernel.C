//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ShaftTimeDerivativeScalarKernel.h"
#include "ShaftConnectableUserObjectInterface.h"
#include "UserObject.h"
#include "MooseVariableScalar.h"
#include "Assembly.h"

registerMooseObject("ThermalHydraulicsApp", ShaftTimeDerivativeScalarKernel);

InputParameters
ShaftTimeDerivativeScalarKernel::validParams()
{
  InputParameters params = ScalarKernel::validParams();
  params.addRequiredParam<std::vector<UserObjectName>>("uo_names",
                                                       "Names of shaft-connectable user objects");

  params.set<MultiMooseEnum>("vector_tags") = "time";
  params.set<MultiMooseEnum>("matrix_tags") = "system time";

  return params;
}

ShaftTimeDerivativeScalarKernel::ShaftTimeDerivativeScalarKernel(const InputParameters & parameters)
  : ScalarKernel(parameters),
    _u_dot(_var.uDot()),
    _du_dot_du(_var.duDotDu()),
    _uo_names(getParam<std::vector<UserObjectName>>("uo_names")),
    _n_components(_uo_names.size())
{
  _shaft_connected_uos.resize(_n_components);
  for (unsigned int i = 0; i < _n_components; ++i)
  {
    _shaft_connected_uos[i] =
        &getUserObjectByName<ShaftConnectableUserObjectInterface>(_uo_names[i]);
  }
}

void
ShaftTimeDerivativeScalarKernel::reinit()
{
}

void
ShaftTimeDerivativeScalarKernel::computeResidual()
{
  prepareVectorTag(_assembly, _var.number());

  Real sum_inertias = 0;
  for (unsigned int i = 0; i < _n_components; ++i)
    sum_inertias += _shaft_connected_uos[i]->getMomentOfInertia();

  _local_re(0) += sum_inertias * _u_dot[0];

  accumulateTaggedLocalResidual();
}

void
ShaftTimeDerivativeScalarKernel::computeJacobian()
{
  prepareMatrixTag(_assembly, _var.number(), _var.number());

  Real sum_inertias = 0;
  for (unsigned int i = 0; i < _n_components; ++i)
    sum_inertias += _shaft_connected_uos[i]->getMomentOfInertia();

  // FIXME: Add dI_domega * domega_dt
  _local_ke(0, 0) += sum_inertias * _du_dot_du[0];

  accumulateTaggedLocalMatrix();

  for (unsigned int i = 0; i < _n_components; ++i)
  {
    DenseMatrix<Real> jacobian_block;
    std::vector<dof_id_type> dofs_j;
    _shaft_connected_uos[i]->getMomentOfInertiaJacobianData(jacobian_block, dofs_j);
    jacobian_block.scale(_u_dot[0]);
    _assembly.cacheJacobianBlock(jacobian_block, _var.dofIndices(), dofs_j, _var.scalingFactor());
  }
}
