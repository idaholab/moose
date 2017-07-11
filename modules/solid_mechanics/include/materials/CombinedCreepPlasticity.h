/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef COMBINEDCREEPPLASTICITY_H
#define COMBINEDCREEPPLASTICITY_H

#include "ConstitutiveModel.h"

class ReturnMappingModel;

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

template <>
InputParameters validParams<CombinedCreepPlasticity>();

#endif // MATERIALDRIVER_H
