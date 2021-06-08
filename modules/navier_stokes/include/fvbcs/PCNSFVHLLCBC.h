//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "FVFluxBC.h"

class SinglePhaseFluidProperties;

/**
 * Base clase for HLLC boundary conditions for the Euler equation
 */
class PCNSFVHLLCBC : public FVFluxBC
{
public:
  PCNSFVHLLCBC(const InputParameters & parameters);

  static InputParameters validParams();

protected:
  virtual ADReal computeQpResidual() override;

  /**
   * this function is a call back for setting quantities for
   * computing wave speed before calling the wave speed routine
   */
  virtual void preComputeWaveSpeed() = 0;

  ///@{ flux functions on elem & from boundary
  virtual ADReal fluxElem() = 0;
  virtual ADReal fluxBoundary() = 0;
  ///@}

  ///@{ HLLC modifications to flux for elem & boundary, see Toro
  virtual ADReal hllcElem() = 0;
  virtual ADReal hllcBoundary() = 0;
  ///@}

  ///@{ conserved variable of this equation from elem and boundary
  virtual ADReal conservedVariableElem() = 0;
  virtual ADReal conservedVariableBoundary() = 0;
  ///@}

  /// fluid properties
  const SinglePhaseFluidProperties & _fluid;

  ///@{ material properties on the elem side of the boundary
  const ADMaterialProperty<Real> & _specific_internal_energy_elem;
  const ADMaterialProperty<RealVectorValue> & _vel_elem;
  const ADMaterialProperty<Real> & _speed_elem;
  const ADMaterialProperty<Real> & _rho_elem;
  const ADMaterialProperty<Real> & _pressure_elem;
  const ADMaterialProperty<Real> & _rho_et_elem;
  const ADMaterialProperty<Real> & _ht_elem;
  const MaterialProperty<Real> & _eps_elem;
  ///@}

  ///@{ the wave speeds
  ADReal _SL;
  ADReal _SM;
  ADReal _SR;
  ///@}

  /// speeds normal to the interface on the element side
  ADReal _normal_speed_elem;

  ///@{ these quantities must be computed in preComputeWaveSpeed
  ADReal _normal_speed_boundary;
  ADReal _rho_boundary;
  ADRealVectorValue _vel_boundary;
  ADReal _specific_internal_energy_boundary;
  ADReal _pressure_boundary;
  ADReal _ht_boundary;
  ADReal _et_boundary;
  ADReal _rho_et_boundary;
  Real _eps_boundary;
  ///@}
};
