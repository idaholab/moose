//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CNSFVHLLCFluidEnergyBC.h"

registerMooseObject("NavierStokesApp", CNSFVHLLCSpecifiedMassFluxAndTemperatureFluidEnergyBC);
registerMooseObject("NavierStokesApp", CNSFVHLLCSpecifiedPressureFluidEnergyBC);

template <typename T>
InputParameters
CNSFVHLLCFluidEnergyBC<T>::validParams()
{
  InputParameters params = T::validParams();
  params.addClassDescription(
      "Implements the fluid energy boundary flux portion of the free-flow HLLC "
      "discretization given specified mass fluxes and fluid temperature");
  return params;
}

template <>
InputParameters
CNSFVHLLCFluidEnergyBC<CNSFVHLLCSpecifiedPressureBC>::validParams()
{
  InputParameters params = CNSFVHLLCSpecifiedPressureBC::validParams();
  params.addClassDescription(
      "Implements the fluid energy boundary flux portion of the free-flow HLLC "
      "discretization given specified pressure");
  return params;
}

template <typename T>
CNSFVHLLCFluidEnergyBC<T>::CNSFVHLLCFluidEnergyBC(const InputParameters & params) : T(params)
{
}

template <typename T>
ADReal
CNSFVHLLCFluidEnergyBC<T>::fluxElem()
{
  return this->_normal_speed_elem * this->_rho_elem[this->_qp] * this->_ht_elem[this->_qp];
}

template <typename T>
ADReal
CNSFVHLLCFluidEnergyBC<T>::fluxBoundary()
{
  return this->_normal_speed_boundary * this->_rho_boundary * this->_ht_boundary;
}

template <typename T>
ADReal
CNSFVHLLCFluidEnergyBC<T>::hllcElem()
{
  return this->_rho_et_elem[this->_qp] / this->_rho_elem[this->_qp] +
         (this->_SM - this->_normal_speed_elem) *
             (this->_SM + this->_pressure_elem[this->_qp] / this->_rho_elem[this->_qp] /
                              (this->_SL - this->_normal_speed_elem));
}

template <typename T>
ADReal
CNSFVHLLCFluidEnergyBC<T>::hllcBoundary()
{
  return this->_rho_et_boundary / this->_rho_boundary +
         (this->_SM - this->_normal_speed_boundary) *
             (this->_SM + this->_pressure_boundary / this->_rho_boundary /
                              (this->_SR - this->_normal_speed_boundary));
}

template <typename T>
ADReal
CNSFVHLLCFluidEnergyBC<T>::conservedVariableElem()
{
  return this->_rho_et_elem[this->_qp];
}

template <typename T>
ADReal
CNSFVHLLCFluidEnergyBC<T>::conservedVariableBoundary()
{
  return this->_rho_et_boundary;
}

template class CNSFVHLLCFluidEnergyBC<CNSFVHLLCSpecifiedMassFluxAndTemperatureBC>;
template class CNSFVHLLCFluidEnergyBC<CNSFVHLLCSpecifiedPressureBC>;
