//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CNSFVHLLCMomentumSpecifiedPressureBC.h"
#include "Function.h"

// Full specialization of the validParams function for this object
registerMooseObject("NavierStokesApp", CNSFVHLLCMomentumSpecifiedPressureBC);

InputParameters
CNSFVHLLCMomentumSpecifiedPressureBC::validParams()
{
  InputParameters params = CNSFVHLLCMomentumImplicitBC::validParams();
  params.addClassDescription("Implements an HLLC boundary condition for the momentum conservation "
                             "equation in which the pressure is specified.");
  params.addRequiredParam<FunctionName>("specified_pressure_function",
                                        "Specified pressure function");
  return params;
}

CNSFVHLLCMomentumSpecifiedPressureBC::CNSFVHLLCMomentumSpecifiedPressureBC(
    const InputParameters & parameters)
  : CNSFVHLLCMomentumImplicitBC(parameters),
    _pressure_function(getFunction("specified_pressure_function"))
{
}

ADReal
CNSFVHLLCMomentumSpecifiedPressureBC::fluxElem()
{
  return _normal_speed_elem * _rho_elem[_qp] * _vel_elem[_qp](_index) +
         _normal(_index) * _pressure_elem[_qp];
}

ADReal
CNSFVHLLCMomentumSpecifiedPressureBC::fluxBoundary()
{
  return _normal_speed_boundary * _rho_boundary * _vel_boundary(_index) +
         _normal(_index) * _pressure_function.value(_t, _face_info->faceCentroid());
}
