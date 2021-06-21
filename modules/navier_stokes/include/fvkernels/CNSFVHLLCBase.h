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

#include <vector>
#include <array>
#include <unordered_map>

class FaceInfo;
class HLLCUserObject;
class SinglePhaseFluidProperties;

/**
 * Helper structure for holding data necessary for computing HLLC fluxes
 */
struct HLLCData
{
  /// fluid properties
  const SinglePhaseFluidProperties & fluid;

  ///@{ densities left == elem, right == neighbor
  const ADReal & rho_elem;
  const ADReal & rho_neighbor;
  ///@}

  ///@{ velocities left == elem, right == neighbor
  const ADRealVectorValue & vel_elem;
  const ADRealVectorValue & vel_neighbor;
  ///@}

  ///@{ internal energies left == elem, right == neighbor
  const ADReal & e_elem;
  const ADReal & e_neighbor;
  ///@}
};

/**
 * Base class for both HLLC inter-cell flux kernels and boundary conditions. This class holds the
 * method that computes the wave speeds necessary for HLLC flux determination
 */
class CNSFVHLLCBase : public FVFluxKernel
{
public:
  static InputParameters validParams();
  CNSFVHLLCBase(const InputParameters & params);

  /// helper function for computing wave speed
  static std::array<ADReal, 3> waveSpeed(const HLLCData & hllc_data,
                                         const ADRealVectorValue & normal);

protected:
  HLLCData hllcData() const;

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

  const ADMaterialProperty<Real> & _rho_et_elem;
  const ADMaterialProperty<Real> & _rho_et_neighbor;

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
};
