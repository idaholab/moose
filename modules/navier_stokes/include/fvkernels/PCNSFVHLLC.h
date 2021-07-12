//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "FVFluxKernel.h"

class HLLCUserObject;
class SinglePhaseFluidProperties;

/**
 * Base class for porous HLLC inter-cell flux kernels. This class defines the interfaces necessary
 * to complete the definition of the porous HLLC fluxes in derived mass, momentum, and fluid energy
 * classes. It also selects the HLLC flux computation branch based on the wave speeds, whose
 * computation is defined in this class
 */
class PCNSFVHLLC : public FVFluxKernel
{
public:
  static InputParameters validParams();
  PCNSFVHLLC(const InputParameters & params);

  /// helper function for computing wave speed
  static std::array<ADReal, 3> waveSpeed(const ADReal & rho_elem,
                                         const ADRealVectorValue & vel_elem,
                                         const ADReal & e_elem,
                                         Real eps_elem,
                                         const ADReal & rho_neighbor,
                                         const ADRealVectorValue & vel_neighbor,
                                         const ADReal & e_neighbor,
                                         Real eps_neighbor,
                                         const SinglePhaseFluidProperties & fluid,
                                         const ADRealVectorValue & normal);

protected:
  virtual ADReal computeQpResidual() override;

  ///@{ flux functions on elem & neighbor, i.e. standard left/right values of F
  virtual ADReal fluxElem() = 0;
  virtual ADReal fluxNeighbor() = 0;
  ///@}

  ///@{ HLLC modifications to flux for elem & neighbor, see Toro
  virtual ADReal hllcElem() = 0;
  virtual ADReal hllcNeighbor() = 0;
  ///@}

  ///@{ conserved variable of this equation This is not just _u_elem/_u_neighbor
  /// to allow using different sets of variables in the future
  virtual ADReal conservedVariableElem() = 0;
  virtual ADReal conservedVariableNeighbor() = 0;
  ///@}

  ///@{ the wave speeds
  ADReal _SL;
  ADReal _SM;
  ADReal _SR;
  ///@}

  ///@{ speeds normal to the interface
  ADReal _normal_speed_elem;
  ADReal _normal_speed_neighbor;
  ///@}

  /// fluid properties
  const SinglePhaseFluidProperties & _fluid;

  ///@{ internal energies left == elem, right == neighbor
  const ADMaterialProperty<Real> & _specific_internal_energy_elem;
  const ADMaterialProperty<Real> & _specific_internal_energy_neighbor;
  ///@}

  ///@{ total energies left == elem, right == neighbor
  const ADMaterialProperty<Real> & _rho_et_elem;
  const ADMaterialProperty<Real> & _rho_et_neighbor;
  ///@}

  ///@{ velocities left == elem, right == neighbor
  const ADMaterialProperty<RealVectorValue> & _vel_elem;
  const ADMaterialProperty<RealVectorValue> & _vel_neighbor;
  ///@}

  ///@{ speeds left == elem, right == neighbor
  const ADMaterialProperty<Real> & _speed_elem;
  const ADMaterialProperty<Real> & _speed_neighbor;
  ///@}

  ///@{ densities left == elem, right == neighbor
  const ADMaterialProperty<Real> & _rho_elem;
  const ADMaterialProperty<Real> & _rho_neighbor;
  ///@}

  ///@{ pressures left == elem, right == neighbor
  const ADMaterialProperty<Real> & _pressure_elem;
  const ADMaterialProperty<Real> & _pressure_neighbor;
  ///@}

  ///@{ porosities left == elem, right == neighbor
  const MaterialProperty<Real> & _eps_elem;
  const MaterialProperty<Real> & _eps_neighbor;
  ///@}
};
