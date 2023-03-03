//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GapFluxModelBase.h"
#include "ModularGapConductanceConstraint.h"

InputParameters
GapFluxModelBase::validParams()
{
  InputParameters params = InterfaceUserObjectBase::validParams();
  params.addClassDescription("Gap flux model base class");

  // UOs of this type should not be executed by MOOSE, but only called through
  // ModularGapConductanceConstraint
  params.set<ExecFlagEnum>("execute_on") = EXEC_CUSTOM;
  params.suppressParameter<ExecFlagEnum>("execute_on");

  // flux models default to operating on the displaced mesh
  params.set<bool>("use_displaced_mesh") = true;

  return params;
}

GapFluxModelBase::GapFluxModelBase(const InputParameters & parameters)
  : InterfaceUserObjectBase(parameters), ADFunctorInterface(this), _qp(0), _gap_width(0.0)
{
}

ADReal
GapFluxModelBase::computeFluxInternal(
    const ModularGapConductanceConstraint & mortar_constraint) const
{
  // Cache general geometry information
  // This allows derived user object to compute gap physics without external dependencies

  _qp = mortar_constraint._qp;
  _gap_width = mortar_constraint._gap_width;
  _surface_integration_factor = mortar_constraint._surface_integration_factor;
  _adjusted_length = mortar_constraint._adjusted_length;
  _normal_pressure = mortar_constraint._normal_pressure;
  _secondary_point = Moose::ElemPointArg({mortar_constraint._interior_secondary_elem,
                                          mortar_constraint._phys_points_secondary[_qp],
                                          false});
  _primary_point = Moose::ElemPointArg({mortar_constraint._interior_primary_elem,
                                        mortar_constraint._phys_points_primary[_qp],
                                        false});

  return computeFlux();
}
