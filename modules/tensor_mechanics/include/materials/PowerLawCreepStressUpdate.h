//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef RECOMPUTERADIALRETURNPOWERLAWCREEP_H
#define RECOMPUTERADIALRETURNPOWERLAWCREEP_H

#include "RadialReturnStressUpdate.h"
#include "MooseMesh.h"

class PowerLawCreepStressUpdate;

template <>
InputParameters validParams<PowerLawCreepStressUpdate>();

/**
 * This class uses the Discrete material in a radial return isotropic creep
 * model.  This class is one of the basic
 * radial return constitutive models; more complex constitutive models combine
 * creep and plasticity.
 *
 * This class inherits from RadialReturnStressUpdate and must be used
 * in conjunction with ComputeReturnMappingStress.  This class calculates
 * creep based on stress, temperature, and time effects.  This class also
 * computes the creep strain as a stateful material property.
 */
class PowerLawCreepStressUpdate : public RadialReturnStressUpdate
{
public:
  PowerLawCreepStressUpdate(const InputParameters & parameters);

protected:
  virtual void initQpStatefulProperties() override;
  virtual void propagateQpStatefulProperties() override;

  virtual void computeStressInitialize(const Real effective_trial_stress,
                                       const RankFourTensor & elasticity_tensor) override;
  virtual void computeStressFinalize(const RankTwoTensor & plasticStrainIncrement) override;

  virtual Real computeResidual(const Real effective_trial_stress, const Real scalar) override;
  virtual Real computeDerivative(const Real effective_trial_stress, const Real scalar) override;

  /// String that is prepended to the creep_strain Material Property
  const std::string _creep_prepend;

  const Real _coefficient;
  const Real _n_exponent;
  const Real _m_exponent;
  const Real _activation_energy;
  const Real _gas_constant;
  const Real _start_time;
  Real _exponential;
  Real _exp_time;
  const bool _has_temp;

  const VariableValue & _temperature;
  MaterialProperty<RankTwoTensor> & _creep_strain;
  const MaterialProperty<RankTwoTensor> & _creep_strain_old;

  Real _max_creep_incr;
};

template <>
InputParameters validParams<PowerLawCreepStressUpdate>();

#endif // RECOMPUTERADIALRETURNPOWERLAWCREEP_H
