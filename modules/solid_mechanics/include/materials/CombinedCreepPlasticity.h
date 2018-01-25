//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef COMBINEDCREEPPLASTICITY_H
#define COMBINEDCREEPPLASTICITY_H

#include "ConstitutiveModel.h"

class ReturnMappingModel;

class CombinedCreepPlasticity;

template <>
InputParameters validParams<CombinedCreepPlasticity>();

/**
 * One or more constitutive models coupled together.
 */

class CombinedCreepPlasticity : public ConstitutiveModel
{
public:
  CombinedCreepPlasticity(const InputParameters & parameters);
  virtual ~CombinedCreepPlasticity() {}

  /// Compute the stress (sigma += deltaSigma)
  virtual void computeStress(const Elem & current_elem,
                             const SymmElasticityTensor & elasticityTensor,
                             const SymmTensor & stress_old,
                             SymmTensor & strain_increment,
                             SymmTensor & stress_new);

  virtual bool modifyStrainIncrement(const Elem & current_elem,
                                     SymmTensor & strain_increment,
                                     SymmTensor & d_strain_dT);

protected:
  virtual void initialSetup();

  std::map<SubdomainID, std::vector<MooseSharedPointer<ReturnMappingModel>>> _submodels;

  unsigned int _max_its;
  bool _output_iteration_info;
  Real _relative_tolerance;
  Real _absolute_tolerance;
  MaterialProperty<Real> & _matl_timestep_limit;

private:
};

#endif // MATERIALDRIVER_H
