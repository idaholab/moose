//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CNSFVHLLCSpecifiedPressureBC.h"
#include "NS.h"
#include "Function.h"
#include "SinglePhaseFluidProperties.h"

InputParameters
CNSFVHLLCSpecifiedPressureBC::validParams()
{
  auto params = CNSFVHLLCBC::validParams();
  params.addRequiredParam<FunctionName>(NS::pressure, "A function for the pressure");
  return params;
}

CNSFVHLLCSpecifiedPressureBC::CNSFVHLLCSpecifiedPressureBC(const InputParameters & parameters)
  : CNSFVHLLCBC(parameters), _pressure_boundary_function(getFunction(NS::pressure))
{
}

void
CNSFVHLLCSpecifiedPressureBC::preComputeWaveSpeed()
{
  _pressure_boundary = _pressure_boundary_function.value(_t, _face_info->faceCentroid());

  // rho and vel implicit -> 1 + n_dim numerical bcs
  _rho_boundary = _rho_elem[_qp];
  _vel_boundary = _vel_elem[_qp];

  _normal_speed_boundary = _normal * _vel_boundary;
  _specific_internal_energy_boundary = _fluid.e_from_p_rho(_pressure_boundary, _rho_boundary);
  _et_boundary = _specific_internal_energy_boundary + 0.5 * _vel_boundary * _vel_boundary;
  _rho_et_boundary = _rho_boundary * _et_boundary;
  _ht_boundary = _et_boundary + _pressure_boundary / _rho_boundary;
}
