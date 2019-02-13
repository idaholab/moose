//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef RADIALRETURNCREEPSTRESSUPDATEBASE_H
#define RADIALRETURNCREEPSTRESSUPDATEBASE_H

#include "RadialReturnStressUpdate.h"

class RadialReturnCreepStressUpdateBase;

template <>
InputParameters validParams<RadialReturnCreepStressUpdateBase>();

/**
 * This class provides baseline functionallity for creep models based on the stress update material
 * in a radial return isotropic creep calculations.
 */
class RadialReturnCreepStressUpdateBase : public RadialReturnStressUpdate
{
public:
  RadialReturnCreepStressUpdateBase(const InputParameters & parameters);

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

#endif // RADIALRETURNCREEPSTRESSUPDATEBASE_H
