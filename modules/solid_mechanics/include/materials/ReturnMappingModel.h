/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef RETURNMAPPINGMODEL_H
#define RETURNMAPPINGMODEL_H

#include "ConstitutiveModel.h"

/**
 * One or more constitutive models coupled together.
 */

class ReturnMappingModel : public ConstitutiveModel
{
public:
  ReturnMappingModel(const InputParameters & parameters);
  virtual ~ReturnMappingModel() {}

  /// Compute the stress (sigma += deltaSigma)
  virtual void computeStress(const Elem & current_elem,
                             unsigned qp,
                             const SymmElasticityTensor & elasticityTensor,
                             const SymmTensor & stress_old,
                             SymmTensor & strain_increment,
                             SymmTensor & stress_new);

  void computeStress(const Elem & /*current_elem*/,
                     unsigned qp,
                     const SymmElasticityTensor & elasticityTensor,
                     const SymmTensor & stress_old,
                     SymmTensor & strain_increment,
                     SymmTensor & stress_new,
                     SymmTensor & inelastic_strain_increment);

protected:
  virtual void computeStressInitialize(unsigned /*qp*/,
                                       Real /*effectiveTrialStress*/,
                                       const SymmElasticityTensor & /*elasticityTensor*/)
  {
  }
  virtual void computeStressFinalize(unsigned /*qp*/,
                                     const SymmTensor & /*inelasticStrainIncrement*/)
  {
  }

  virtual void iterationInitialize(unsigned /*qp*/, Real /*scalar*/) {}
  virtual Real computeResidual(unsigned qp, Real effectiveTrialStress, Real scalar) = 0;
  virtual Real computeDerivative(unsigned qp, Real effectiveTrialStress, Real scalar) = 0;
  virtual void iterationFinalize(unsigned /*qp*/, Real /*scalar*/) {}

  const unsigned int _max_its;
  const bool _output_iteration_info;
  const bool _output_iteration_info_on_error;
  const Real _relative_tolerance;
  const Real _absolute_tolerance;
  Real _effective_strain_increment;

private:
};

template <>
InputParameters validParams<ReturnMappingModel>();

#endif // RETURNMAPPINGMODEL_H
