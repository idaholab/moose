//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef CRYSTALPLASTICITYSTATEVARRATECOMPONENTVOCE_H
#define CRYSTALPLASTICITYSTATEVARRATECOMPONENTVOCE_H

#include "CrystalPlasticityStateVarRateComponent.h"
#include "MooseTypes.h"

class CrystalPlasticityStateVarRateComponentVoce;

template <>
InputParameters validParams<CrystalPlasticityStateVarRateComponentVoce>();

/**
 * Phenomenological constitutive model state variable evolution rate component userobject class.
 */
class CrystalPlasticityStateVarRateComponentVoce : public CrystalPlasticityStateVarRateComponent
{

public:
  CrystalPlasticityStateVarRateComponentVoce(const InputParameters & parameters);

<<<<<<< HEAD
  /// computing the slip system hardening rate
  virtual bool calcStateVariableEvolutionRateComponent(unsigned int qp,
                                                       std::vector<Real> & val) const;

  /// class for switching between different crystal lattice types
  static MooseEnum crystalLatticeTypeOptions();

=======
  // computing the slip system hardening rate
  virtual bool calcStateVariableEvolutionRateComponent(unsigned int qp,
                                                       std::vector<Real> & val) const;

  // class containgn the avialbale slip systems
  static MooseEnum crystalLatticeTypeOptions();
<<<<<<< HEAD
>>>>>>> Voce Hardening Law for the crystal plasticity user object based framework
=======

>>>>>>> close #11307
protected:
  const MaterialProperty<std::vector<Real>> & _mat_prop_slip_rate;
  const MaterialProperty<std::vector<Real>> & _mat_prop_state_var;

<<<<<<< HEAD
<<<<<<< HEAD
<<<<<<< HEAD
<<<<<<< HEAD
  /// the variable to switch crystal lattice type (i.e. FCC or BCC)
  MooseEnum _crystal_lattice_type;

  /// the vectors of the input paramters
=======

=======
>>>>>>> close #11307
  // the switching variable containing the type of crystal (i.e. FCC12 or BCC12)
=======

  // the switching variable containing the type of crystal (i.e. FCC or BCC)
>>>>>>> Voce Hardening Law for the crystal plasticity user object based framework
=======
  // the switching variable containing the type of crystal (i.e. FCC12 or BCC12)
>>>>>>> close #11307
  MooseEnum _crystal_lattice_type;

  // teh vectors containing the input paramters
>>>>>>> Voce Hardening Law for the crystal plasticity user object based framework
  std::vector<unsigned int> _groups;
  std::vector<Real> _h0_group_values;
  std::vector<Real> _tau0_group_values;
  std::vector<Real> _tauSat_group_values;
  std::vector<Real> _hardeningExponent_group_values;
  std::vector<Real> _selfHardening_group_values;
  std::vector<Real> _coplanarHardening_group_values;
  std::vector<Real> _GroupGroup_Hardening_group_values;

<<<<<<< HEAD
  /// the number of slip system groups
  unsigned int _n_groups;
  /// the vector associating a slip system to its slip plane ID
  std::vector<unsigned int> _slipSystem_PlaneID;
  /// the vector associating a slip system to its groud ID
  std::vector<unsigned int> _slipSystem_GroupID;

  /// method checking the input paramters
  virtual void checkHardeningParametersSize() const;
  /// method associating slip system to their slip plane
  virtual void initSlipSystem_GroupID(std::vector<unsigned int> & _slipSystem_GroupID) const;
  /// method associating slip system to their group
  virtual void initSlipSystem_PlaneID(std::vector<unsigned int> & _slipSystem_PlaneID) const;
  /// method retriving the appropiate self/latent hardening coefficient
  virtual Real getHardeningCoefficient(unsigned int slipSystemIndex_i,
                                       unsigned int slipSystemIndex_j) const;
=======
  // the number of slip system groups
  unsigned int _n_groups;
  // the vector associating a slip system to its slip plane ID
  std::vector<unsigned int> _slipSystem_PlaneID;
  // the vector associating a slip system to its groud ID
  std::vector<unsigned int> _slipSystem_GroupID;

  // method checking the input paramters
  virtual void checkHardeningParametersSize() const;
  // method assocaiting slip system to their slip plane
  virtual void initSlipSystem_GroupID(std::vector<unsigned int> & _slipSystem_GroupID) const;
  // method assocaiting slip system to their group
  virtual void initSlipSystem_PlaneID(std::vector<unsigned int> & _slipSystem_PlaneID) const;
  // method retriving the appropiate self/latent hardening coefficient
<<<<<<< HEAD
  virtual Real getHardeningCoefficient(unsigned int slipSystemIndex_i, unsigned int slipSystemIndex_j) const;
>>>>>>> Voce Hardening Law for the crystal plasticity user object based framework
=======
  virtual Real getHardeningCoefficient(unsigned int slipSystemIndex_i,
                                       unsigned int slipSystemIndex_j) const;
>>>>>>> close #11307
};

#endif // CRYSTALPLASTICITYSTATEVARRATECOMPONENTVOCE_H
