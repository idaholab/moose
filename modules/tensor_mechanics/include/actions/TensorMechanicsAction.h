//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef TENSORMECHANICSACTION_H
#define TENSORMECHANICSACTION_H

#include "TensorMechanicsActionBase.h"

class TensorMechanicsAction;

template <>
InputParameters validParams<TensorMechanicsAction>();

class TensorMechanicsAction : public TensorMechanicsActionBase
{
public:
  TensorMechanicsAction(const InputParameters & params);

  virtual void act();

protected:
  void actSubdomainChecks();
  void actOutputGeneration();
  void actGatherActionParameters();

  virtual std::string getKernelType();
  virtual InputParameters getKernelParameters(std::string type);

  ///@{ displacement variables
  std::vector<NonlinearVariableName> _displacements;
  unsigned int _ndisp;
  std::vector<VariableName> _coupled_displacements;
  ///@}

  ///@{ residual debugging
  std::vector<AuxVariableName> _save_in;
  std::vector<AuxVariableName> _diag_save_in;
  ///@}

  Moose::CoordinateSystemType _coord_system;

  /// if this vector is not empty the variables, kernels and materials are restricted to these subdomains
  std::vector<SubdomainName> _subdomain_names;

  /// set generated from the passed in vector of subdomain names
  std::set<SubdomainID> _subdomain_ids;

  /// set generated from the combined block restrictions of all TensorMechanics/Master action blocks
  std::set<SubdomainID> _subdomain_id_union;

  /// strain formulation
  enum class Strain
  {
    Small,
    Finite
  } _strain;

  /// strain formulation
  enum class StrainAndIncrement
  {
    SmallTotal,
    FiniteTotal,
    SmallIncremental,
    FiniteIncremental
  } _strain_and_increment;

  /// use an out of plane stress/strain formulation
  enum class PlanarFormulation
  {
    None,
    PlaneStrain,
    GeneralizedPlaneStrain,
    /* PlaneStress */
  } _planar_formulation;

  /// use displaced mesh (true unless _strain is SMALL)
  bool _use_displaced_mesh;

  /// output aux variables to generate for sclar stress/strain tensor quantities
  std::vector<std::string> _generate_output;
};

#endif // TENSORMECHANICSACTION_H
