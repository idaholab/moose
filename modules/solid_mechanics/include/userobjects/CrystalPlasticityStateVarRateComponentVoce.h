//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "CrystalPlasticityStateVarRateComponent.h"
#include "MooseTypes.h"

/**
 * Phenomenological constitutive model state variable evolution rate component userobject class.
 */
class CrystalPlasticityStateVarRateComponentVoce : public CrystalPlasticityStateVarRateComponent
{
public:
  static InputParameters validParams();

  CrystalPlasticityStateVarRateComponentVoce(const InputParameters & parameters);

  /// computing the slip system hardening rate
  virtual bool calcStateVariableEvolutionRateComponent(unsigned int qp,
                                                       std::vector<Real> & val) const;

  /// class for switching between different crystal lattice types
  static MooseEnum crystalLatticeTypeOptions();

protected:
  const MaterialProperty<std::vector<Real>> & _mat_prop_slip_rate;
  const MaterialProperty<std::vector<Real>> & _mat_prop_state_var;

  /// the variable to switch crystal lattice type (i.e. FCC or BCC)
  MooseEnum _crystal_lattice_type;

  ///@{ the vectors of the input paramters
  std::vector<unsigned int> _groups;
  std::vector<Real> _h0_group_values;
  std::vector<Real> _tau0_group_values;
  std::vector<Real> _tauSat_group_values;
  std::vector<Real> _hardeningExponent_group_values;
  std::vector<Real> _selfHardening_group_values;
  std::vector<Real> _coplanarHardening_group_values;
  std::vector<Real> _GroupGroup_Hardening_group_values;
  ///@}

  /// the number of slip system groups
  const unsigned int _n_groups;

  /// the vector associating a slip system to its slip plane ID
  std::vector<unsigned int> _slipSystem_PlaneID;
  /// the vector associating a slip system to its groud ID
  std::vector<unsigned int> _slipSystem_GroupID;

  /// method associating slip system to their group by generating a vector
  /// containing the association between slip system number and slip plane number
  virtual void initSlipSystemPlaneID(std::vector<unsigned int> & _slipSystem_PlaneID) const;

  /// method associating slip system to their slip plane by generating a vector
  /// containing the association between slip system number and provided group edges
  virtual void initSlipSystemGroupID(std::vector<unsigned int> & _slipSystem_GroupID) const;

  /// method retriving the appropiate self/latent hardening coefficient
  virtual Real getHardeningCoefficient(unsigned int slipSystemIndex_i,
                                       unsigned int slipSystemIndex_j) const;
};
