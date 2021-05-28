//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PCNSFVHLLCMassBC.h"

registerMooseObject("NavierStokesApp", PCNSFVHLLCSpecifiedMassFluxAndTemperatureMassBC);
registerMooseObject("NavierStokesApp", PCNSFVHLLCSpecifiedPressureMassBC);

template <typename T>
InputParameters
PCNSFVHLLCMassBC<T>::validParams()
{
  InputParameters params = T::validParams();
  params.addClassDescription("Implements the mass boundary flux portion of the porous HLLC "
                             "discretization given specified mass fluxes and fluid temperature");
  return params;
}

template <>
InputParameters
PCNSFVHLLCMassBC<PCNSFVHLLCSpecifiedPressureBC>::validParams()
{
  InputParameters params = PCNSFVHLLCSpecifiedPressureBC::validParams();
  params.addClassDescription("Implements the mass boundary flux portion of the porous HLLC "
                             "discretization given specified pressure");
  return params;
}

template <typename T>
PCNSFVHLLCMassBC<T>::PCNSFVHLLCMassBC(const InputParameters & params) : T(params)
{
}

template <typename T>
ADReal
PCNSFVHLLCMassBC<T>::fluxElem()
{
  return this->_normal_speed_elem * this->_eps_elem[this->_qp] * this->_rho_elem[this->_qp];
}

template <typename T>
ADReal
PCNSFVHLLCMassBC<T>::fluxBoundary()
{
  return this->_normal_speed_boundary * this->_eps_boundary * this->_rho_boundary;
}

template <typename T>
ADReal
PCNSFVHLLCMassBC<T>::hllcElem()
{
  return 1;
}

template <typename T>
ADReal
PCNSFVHLLCMassBC<T>::hllcBoundary()
{
  return 1;
}

template <typename T>
ADReal
PCNSFVHLLCMassBC<T>::conservedVariableElem()
{
  return this->_eps_elem[this->_qp] * this->_rho_elem[this->_qp];
}

template <typename T>
ADReal
PCNSFVHLLCMassBC<T>::conservedVariableBoundary()
{
  return this->_eps_boundary * this->_rho_boundary;
}

template class PCNSFVHLLCMassBC<PCNSFVHLLCSpecifiedMassFluxAndTemperatureBC>;
template class PCNSFVHLLCMassBC<PCNSFVHLLCSpecifiedPressureBC>;
