//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CNSFVHLLCMassBC.h"

registerMooseObject("NavierStokesApp", CNSFVHLLCSpecifiedMassFluxAndTemperatureMassBC);
registerMooseObject("NavierStokesApp", CNSFVHLLCSpecifiedPressureMassBC);

template <typename T>
InputParameters
CNSFVHLLCMassBC<T>::validParams()
{
  InputParameters params = T::validParams();
  params.addClassDescription("Implements the mass boundary flux portion of the free-flow HLLC "
                             "discretization given specified mass fluxes and fluid temperature");
  return params;
}

template <>
InputParameters
CNSFVHLLCMassBC<CNSFVHLLCSpecifiedPressureBC>::validParams()
{
  InputParameters params = CNSFVHLLCSpecifiedPressureBC::validParams();
  params.addClassDescription("Implements the mass boundary flux portion of the free-flow HLLC "
                             "discretization given specified pressure");
  return params;
}

template <typename T>
CNSFVHLLCMassBC<T>::CNSFVHLLCMassBC(const InputParameters & params) : T(params)
{
}

template <typename T>
ADReal
CNSFVHLLCMassBC<T>::fluxElem()
{
  return this->_normal_speed_elem * this->_rho_elem[this->_qp];
}

template <typename T>
ADReal
CNSFVHLLCMassBC<T>::fluxBoundary()
{
  return this->_normal_speed_boundary * this->_rho_boundary;
}

template <typename T>
ADReal
CNSFVHLLCMassBC<T>::hllcElem()
{
  return 1;
}

template <typename T>
ADReal
CNSFVHLLCMassBC<T>::hllcBoundary()
{
  return 1;
}

template <typename T>
ADReal
CNSFVHLLCMassBC<T>::conservedVariableElem()
{
  return this->_rho_elem[this->_qp];
}

template <typename T>
ADReal
CNSFVHLLCMassBC<T>::conservedVariableBoundary()
{
  return this->_rho_boundary;
}

template class CNSFVHLLCMassBC<CNSFVHLLCSpecifiedMassFluxAndTemperatureBC>;
template class CNSFVHLLCMassBC<CNSFVHLLCSpecifiedPressureBC>;
