//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CZMComputeGlobalTractionBase.h"

InputParameters
CZMComputeGlobalTractionBase::validParams()
{
  InputParameters params = InterfaceMaterial::validParams();

  params.addClassDescription(
      "Base class for computing the equilibrium traction and its derivatives.");
  params.suppressParameter<bool>("use_displaced_mesh");
  return params;
}

CZMComputeGlobalTractionBase::CZMComputeGlobalTractionBase(const InputParameters & parameters)
  : InterfaceMaterial(parameters),
    _traction_global(declareProperty<RealVectorValue>("traction_global")),
    _interface_traction(getMaterialProperty<RealVectorValue>("interface_traction")),
    _dtraction_djump_global(declareProperty<RankTwoTensor>("dtraction_djump_global")),
    _dinterface_traction_djump(getMaterialProperty<RankTwoTensor>("dinterface_traction_djump")),
    _Q0(getMaterialProperty<RealTensorValue>("czm_reference_rotation"))
{
}

void
CZMComputeGlobalTractionBase::computeQpProperties()
{
  // rotate local traction and derivatives to the global coordinate system
  computeEquilibriumTracionAndDerivatives();
}
