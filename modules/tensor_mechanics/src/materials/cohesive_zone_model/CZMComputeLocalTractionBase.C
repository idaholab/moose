//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "Assembly.h"
#include "CZMComputeLocalTractionBase.h"

InputParameters
CZMComputeLocalTractionBase::validParams()
{
  InputParameters params = InterfaceMaterial::validParams();

  params.addClassDescription("Base class for implementing cohesive zone constituive material "
                             "models that can be formulated using the total displacement jump");
  params.addRequiredCoupledVar("displacements",
                               "The string of displacements suitable for the problem statement");
  params.suppressParameter<bool>("use_displaced_mesh");
  return params;
}

CZMComputeLocalTractionBase::CZMComputeLocalTractionBase(const InputParameters & parameters)
  : InterfaceMaterial(parameters),
    _interface_traction(declareProperty<RealVectorValue>("interface_traction")),
    _dinterface_traction_djump(declareProperty<RankTwoTensor>("dinterface_traction_djump")),
    _interface_displacement_jump(
        getMaterialProperty<RealVectorValue>("interface_displacement_jump"))
{
}

void
CZMComputeLocalTractionBase::computeQpProperties()
{
  computeInterfaceTractionAndDerivatives();
}
