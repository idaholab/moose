//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "RadialReturnStressUpdate.h"

/**
 * This class provides baseline functionallity for creep models based on the stress update material
 * in a radial return isotropic creep calculations.
 */
class RadialReturnCreepStressUpdateBase : public RadialReturnStressUpdate
{
public:
  static InputParameters validParams();

  RadialReturnCreepStressUpdateBase(const InputParameters & parameters);

  /**
   * Compute the strain energy rate density for this inelastic model for the current step.
   * @param stress The stress tensor at the end of the step
   * @param strain_rate The strain rate at the end of the step
   * @return The computed strain energy rate density
   */
  virtual Real
  computeStrainEnergyRateDensity(const MaterialProperty<RankTwoTensor> & /*stress*/,
                                 const MaterialProperty<RankTwoTensor> & /*strain_rate*/)
  {
    mooseError(
        "The computation of strain energy rate density needs to be implemented by a child class");
  }

protected:
  virtual void initQpStatefulProperties() override;
  virtual void propagateQpStatefulProperties() override;
  virtual void computeStressFinalize(const RankTwoTensor & plastic_strain_increment) override;

  /**
   * This method returns the derivative of the creep strain with respect to the von mises stress. It
   * assumes the stress delta (von mises stress used to determine the creep rate) is calculated as:
   * effective_trial_stress - _three_shear_modulus * scalar
   */
  virtual Real computeStressDerivative(const Real effective_trial_stress,
                                       const Real scalar) override;

  /*
   * Method that determines the tangent calculation method. For creep only models, the tangent
   * calculation method is always PARTIAL
   */
  virtual TangentCalculationMethod getTangentCalculationMethod() override
  {
    return TangentCalculationMethod::PARTIAL;
  }

  /// String that is prepended to the creep_strain Material Property
  const std::string _creep_prepend;

  /// Creep strain material property
  MaterialProperty<RankTwoTensor> & _creep_strain;
  const MaterialProperty<RankTwoTensor> & _creep_strain_old;
};
