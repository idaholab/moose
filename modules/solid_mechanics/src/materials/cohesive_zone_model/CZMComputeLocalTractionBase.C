//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
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

  params.addClassDescription("Base class for implementing cohesive zone constitutive material "
                             "models that can be formulated using the total displacement jump");
  params.addRequiredCoupledVar("displacements",
                               "The string of displacements suitable for the problem statement");
  params.suppressParameter<bool>("use_displaced_mesh");
  params.addParam<std::string>("base_name", "Material property base name");
  return params;
}

CZMComputeLocalTractionBase::CZMComputeLocalTractionBase(const InputParameters & parameters)
  : InterfaceMaterial(parameters),
    _base_name(isParamValid("base_name") && !getParam<std::string>("base_name").empty()
                   ? getParam<std::string>("base_name") + "_"
                   : ""),
    _interface_traction(declarePropertyByName<RealVectorValue>(_base_name + "interface_traction")),
    _dinterface_traction_djump(
        declarePropertyByName<RankTwoTensor>(_base_name + "dinterface_traction_djump")),
    _interface_displacement_jump(
        getMaterialPropertyByName<RealVectorValue>(_base_name + "interface_displacement_jump"))
{
}

void
CZMComputeLocalTractionBase::initQpStatefulProperties()
{
  _interface_traction[_qp] = 0;
}

void
CZMComputeLocalTractionBase::computeQpProperties()
{
  computeInterfaceTractionAndDerivatives();
}
