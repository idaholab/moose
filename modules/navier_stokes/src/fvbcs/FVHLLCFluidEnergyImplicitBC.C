//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FVHLLCFluidEnergyImplicitBC.h"

namespace nms = NS;

// Full specialization of the validParams function for this object
registerADMooseObject("NavierStokesApp", CNSFVHLLCFluidEnergyImplicitBC);

InputParameters
CNSFVHLLCFluidEnergyImplicitBC::validParams()
{
  InputParameters params =CNSFVHLLCImplicitBC::validParams();
  return params;
}

CNSFVHLLCFluidEnergyImplicitBC::CNSFVHLLCFluidEnergyImplicitBC(const InputParameters & parameters)
  : CNSFVHLLCImplicitBC(parameters),
    _ht_elem(getADMaterialProperty<Real>(nms::specific_total_enthalpy))
{
}

ADReal
CNSFVHLLCFluidEnergyImplicitBC::fluxElem()
{
  return _normal_speed_elem * _rho_elem[_qp] * _ht_elem[_qp];
}

ADReal
CNSFVHLLCFluidEnergyImplicitBC::hllcElem()
{
  return _rho_et_elem[_qp] / _rho_elem[_qp] + (_SM - _normal_speed_elem) * (_SM + _pressure_elem[_qp] / _rho_elem[_qp] /
                                                                (_SL - _normal_speed_elem));
}

ADReal
CNSFVHLLCFluidEnergyImplicitBC::conservedVariableElem()
{
  return _rho_et_elem[_qp];
}
