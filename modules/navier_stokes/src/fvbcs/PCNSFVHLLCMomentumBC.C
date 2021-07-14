//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PCNSFVHLLCMomentumBC.h"

registerMooseObject("NavierStokesApp", PCNSFVHLLCSpecifiedMassFluxAndTemperatureMomentumBC);
registerMooseObject("NavierStokesApp", PCNSFVHLLCSpecifiedPressureMomentumBC);

template <typename T>
void
PCNSFVHLLCMomentumBC<T>::addCommonParams(InputParameters & params)
{
  MooseEnum momentum_component("x=0 y=1 z=2");
  params.addRequiredParam<MooseEnum>(
      "momentum_component",
      momentum_component,
      "The component of the momentum equation that this kernel applies to.");
}

template <typename T>
InputParameters
PCNSFVHLLCMomentumBC<T>::validParams()
{
  InputParameters params = T::validParams();
  addCommonParams(params);
  params.addClassDescription("Implements the momentum boundary flux portion of the porous HLLC "
                             "discretization given specified mass fluxes and fluid temperature");
  return params;
}

template <>
InputParameters
PCNSFVHLLCMomentumBC<PCNSFVHLLCSpecifiedPressureBC>::validParams()
{
  InputParameters params = PCNSFVHLLCSpecifiedPressureBC::validParams();
  addCommonParams(params);
  params.addClassDescription("Implements the momentum boundary flux portion of the porous HLLC "
                             "discretization given specified pressure");
  return params;
}

template <typename T>
PCNSFVHLLCMomentumBC<T>::PCNSFVHLLCMomentumBC(const InputParameters & params)
  : T(params), _index(this->template getParam<MooseEnum>("momentum_component"))
{
}

template <typename T>
ADReal
PCNSFVHLLCMomentumBC<T>::fluxElem()
{
  return this->_normal_speed_elem * this->_eps_elem[this->_qp] * this->_rho_elem[this->_qp] *
             this->_vel_elem[this->_qp](_index) +
         this->_normal(_index) * this->_eps_elem[this->_qp] * this->_pressure_elem[this->_qp];
}

template <typename T>
ADReal
PCNSFVHLLCMomentumBC<T>::fluxBoundary()
{
  return this->_normal_speed_boundary * this->_eps_boundary * this->_rho_boundary *
             this->_vel_boundary(_index) +
         this->_normal(_index) * this->_eps_boundary * this->_pressure_boundary;
}

template <typename T>
ADReal
PCNSFVHLLCMomentumBC<T>::hllcElem()
{
  auto vel_nonnormal = this->_vel_elem[this->_qp] - this->_normal_speed_elem * this->_normal;
  return this->_normal(_index) * this->_SM + vel_nonnormal(_index);

  // For some reason, the below expression doesn't give as good results as the
  // above one.
  // return this->_normal(_index) * (_SM - this->_normal_speed_elem) +
  // this->_vel_elem[this->_qp](_index);
}

template <typename T>
ADReal
PCNSFVHLLCMomentumBC<T>::hllcBoundary()
{
  auto vel_nonnormal = this->_vel_boundary - this->_normal_speed_boundary * this->_normal;
  return this->_normal(_index) * this->_SM + vel_nonnormal(_index);

  // For some reason, the below expression doesn't give as good results as the
  // above one.
  // return this->_normal(_index) * (_SM - this->_normal_speed_boundary) +
  // this->_vel_elem[this->_qp](_index);
}

template <typename T>
ADReal
PCNSFVHLLCMomentumBC<T>::conservedVariableElem()
{
  return this->_eps_elem[this->_qp] * this->_rho_elem[this->_qp] *
         this->_vel_elem[this->_qp](_index);
}

template <typename T>
ADReal
PCNSFVHLLCMomentumBC<T>::conservedVariableBoundary()
{
  return this->_eps_boundary * this->_rho_boundary * this->_vel_boundary(_index);
}

template class PCNSFVHLLCMomentumBC<PCNSFVHLLCSpecifiedMassFluxAndTemperatureBC>;
template class PCNSFVHLLCMomentumBC<PCNSFVHLLCSpecifiedPressureBC>;
