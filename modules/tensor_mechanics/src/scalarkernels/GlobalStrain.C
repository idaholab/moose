//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GlobalStrain.h"
#include "GlobalStrainUserObjectInterface.h"

// MOOSE includes
#include "Assembly.h"
#include "MooseVariableScalar.h"
#include "SystemBase.h"
#include "RankTwoTensor.h"
#include "RankFourTensor.h"

registerMooseObject("TensorMechanicsApp", GlobalStrain);

InputParameters
GlobalStrain::validParams()
{
  InputParameters params = ScalarKernel::validParams();
  params.addClassDescription("Scalar Kernel to solve for the global strain");
  params.addRequiredParam<UserObjectName>("global_strain_uo",
                                          "The name of the GlobalStrainUserObject");

  return params;
}

GlobalStrain::GlobalStrain(const InputParameters & parameters)
  : ScalarKernel(parameters),
    _pst(getUserObject<GlobalStrainUserObjectInterface>("global_strain_uo")),
    _pst_residual(_pst.getResidual()),
    _pst_jacobian(_pst.getJacobian()),
    _periodic_dir(_pst.getPeriodicDirections()),
    _components(_var.order()),
    _dim(_mesh.dimension())
{
  if ((_dim == 1 && _var.order() != FIRST) || (_dim == 2 && _var.order() != THIRD) ||
      (_dim == 3 && _var.order() != SIXTH))
    mooseError("PerdiodicStrain ScalarKernel is only compatible with scalar variables of order "
               "FIRST in 1D, THIRD in 2D, and SIXTH in 3D. Please change the order of the scalar"
               "variable according to the mesh dimension.");

  assignComponentIndices(_var.order());
}

void
GlobalStrain::computeResidual()
{
  prepareVectorTag(_assembly, _var.number());
  for (_i = 0; _i < _local_re.size(); ++_i)
  {
    if (_periodic_dir(_components[_i].first) || _periodic_dir(_components[_i].second))
      _local_re(_i) += _pst_residual(_components[_i].first, _components[_i].second);
  }
  accumulateTaggedLocalResidual();
}

void
GlobalStrain::computeJacobian()
{
  prepareMatrixTag(_assembly, _var.number(), _var.number());
  for (_i = 0; _i < _local_ke.m(); ++_i)
    for (_j = 0; _j < _local_ke.m(); ++_j)
      // periodic direction check is not done for jacobian calculations to avoid zero pivot error
      _local_ke(_i, _j) += _pst_jacobian(_components[_i].first,
                                         _components[_i].second,
                                         _components[_j].first,
                                         _components[_j].second);
  accumulateTaggedLocalMatrix();
}

void
GlobalStrain::assignComponentIndices(Order order)
{
  switch (order)
  {
    case 1:
      _components[0].first = 0;
      _components[0].second = 0;
      break;

    case 3:
      _components[0].first = 0;
      _components[0].second = 0;
      _components[1].first = 1;
      _components[1].second = 1;
      _components[2].first = 0;
      _components[2].second = 1;
      break;

    case 6:
      _components[0].first = 0;
      _components[0].second = 0;
      _components[1].first = 1;
      _components[1].second = 1;
      _components[2].first = 2;
      _components[2].second = 2;
      _components[3].first = 1;
      _components[3].second = 2;
      _components[4].first = 0;
      _components[4].second = 2;
      _components[5].first = 0;
      _components[5].second = 1;
      break;

    default:
      mooseError("PerdiodicStrain ScalarKernel is only compatible with FIRST, THIRD, and SIXTH "
                 "order scalar variables.");
  }
}
