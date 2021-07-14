//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CNSFVHLLCFluidEnergyStagnationInletBC.h"
#include "NS.h"

// Full specialization of the validParams function for this object
registerADMooseObject("NavierStokesApp", CNSFVHLLCFluidEnergyStagnationInletBC);

InputParameters
CNSFVHLLCFluidEnergyStagnationInletBC::validParams()
{
  InputParameters params = CNSFVHLLCStagnationInletBC::validParams();
  params.addClassDescription("Adds the boundary fluid energy flux for HLLC when provided "
                             "stagnation temperature and pressure");
  return params;
}

CNSFVHLLCFluidEnergyStagnationInletBC::CNSFVHLLCFluidEnergyStagnationInletBC(
    const InputParameters & parameters)
  : CNSFVHLLCStagnationInletBC(parameters),
    _ht_elem(getADMaterialProperty<Real>(NS::specific_total_enthalpy))
{
}

ADReal
CNSFVHLLCFluidEnergyStagnationInletBC::fluxElem()
{
  return _normal_speed_elem * _rho_elem[_qp] * _ht_elem[_qp];
}

ADReal
CNSFVHLLCFluidEnergyStagnationInletBC::fluxBoundary()
{
  return _normal_speed_boundary * _rho_boundary * _ht_boundary;
}

ADReal
CNSFVHLLCFluidEnergyStagnationInletBC::hllcElem()
{
  return _specific_internal_energy_elem[_qp] +
         (_SM - _normal_speed_elem) *
             (_SM + _pressure_elem[_qp] / _rho_elem[_qp] / (_SL - _normal_speed_elem));
}

ADReal
CNSFVHLLCFluidEnergyStagnationInletBC::hllcBoundary()
{
  return _specific_internal_energy_boundary +
         (_SM - _normal_speed_boundary) *
             (_SM + _p_boundary / _rho_boundary / (_SL - _normal_speed_boundary));
}

ADReal
CNSFVHLLCFluidEnergyStagnationInletBC::conservedVariableElem()
{
  return _specific_internal_energy_elem[_qp] * _rho_elem[_qp];
}

ADReal
CNSFVHLLCFluidEnergyStagnationInletBC::conservedVariableBoundary()
{
  return _specific_internal_energy_boundary * _rho_boundary;
}
