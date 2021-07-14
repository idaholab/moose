//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PCNSFVHLLCFluidEnergyBC.h"

registerMooseObject("NavierStokesApp", PCNSFVHLLCSpecifiedMassFluxAndTemperatureFluidEnergyBC);
registerMooseObject("NavierStokesApp", PCNSFVHLLCSpecifiedPressureFluidEnergyBC);

template <typename T>
InputParameters
PCNSFVHLLCFluidEnergyBC<T>::validParams()
{
  InputParameters params = T::validParams();
  params.addClassDescription("Implements the fluid energy boundary flux portion of the porous HLLC "
                             "discretization given specified mass fluxes and fluid temperature");
  return params;
}

template <>
InputParameters
PCNSFVHLLCFluidEnergyBC<PCNSFVHLLCSpecifiedPressureBC>::validParams()
{
  InputParameters params = PCNSFVHLLCSpecifiedPressureBC::validParams();
  params.addClassDescription("Implements the fluid energy boundary flux portion of the porous HLLC "
                             "discretization given specified pressure");
  return params;
}

template <typename T>
PCNSFVHLLCFluidEnergyBC<T>::PCNSFVHLLCFluidEnergyBC(const InputParameters & params) : T(params)
{
}

template <typename T>
ADReal
PCNSFVHLLCFluidEnergyBC<T>::fluxElem()
{
  return this->_normal_speed_elem * this->_eps_elem[this->_qp] * this->_rho_elem[this->_qp] *
         this->_ht_elem[this->_qp];
}

template <typename T>
ADReal
PCNSFVHLLCFluidEnergyBC<T>::fluxBoundary()
{
  return this->_normal_speed_boundary * this->_eps_boundary * this->_rho_boundary *
         this->_ht_boundary;
}

template <typename T>
ADReal
PCNSFVHLLCFluidEnergyBC<T>::hllcElem()
{
  return this->_rho_et_elem[this->_qp] / this->_rho_elem[this->_qp] +
         (this->_SM - this->_normal_speed_elem) *
             (this->_SM + this->_pressure_elem[this->_qp] / this->_rho_elem[this->_qp] /
                              (this->_SL - this->_normal_speed_elem));
}

template <typename T>
ADReal
PCNSFVHLLCFluidEnergyBC<T>::hllcBoundary()
{
  return this->_rho_et_boundary / this->_rho_boundary +
         (this->_SM - this->_normal_speed_boundary) *
             (this->_SM + this->_pressure_boundary / this->_rho_boundary /
                              (this->_SR - this->_normal_speed_boundary));
}

template <typename T>
ADReal
PCNSFVHLLCFluidEnergyBC<T>::conservedVariableElem()
{
  return this->_eps_elem[this->_qp] * this->_rho_et_elem[this->_qp];
}

template <typename T>
ADReal
PCNSFVHLLCFluidEnergyBC<T>::conservedVariableBoundary()
{
  return this->_eps_boundary * this->_rho_et_boundary;
}

template class PCNSFVHLLCFluidEnergyBC<PCNSFVHLLCSpecifiedMassFluxAndTemperatureBC>;
template class PCNSFVHLLCFluidEnergyBC<PCNSFVHLLCSpecifiedPressureBC>;
