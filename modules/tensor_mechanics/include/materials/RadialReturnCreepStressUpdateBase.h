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
template <bool is_ad>
class RadialReturnCreepStressUpdateBaseTempl : public RadialReturnStressUpdateTempl<is_ad>
{
public:
  static InputParameters validParams();

  RadialReturnCreepStressUpdateBaseTempl(const InputParameters & parameters);

  using Material::_qp;
  using RadialReturnStressUpdateTempl<is_ad>::propagateQpStatefulPropertiesRadialReturn;
  using SingleVariableReturnMappingSolutionTempl<is_ad>::computeDerivative;

protected:
  virtual void initQpStatefulProperties() override;
  virtual void propagateQpStatefulProperties() override;
  virtual void
  computeStressFinalize(const GenericRankTwoTensor<is_ad> & plastic_strain_increment) override;

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

  /// Creep strain material property
  GenericMaterialProperty<RankTwoTensor, is_ad> & _creep_strain;
  const MaterialProperty<RankTwoTensor> & _creep_strain_old;
};

typedef RadialReturnCreepStressUpdateBaseTempl<false> RadialReturnCreepStressUpdateBase;
typedef RadialReturnCreepStressUpdateBaseTempl<true> ADRadialReturnCreepStressUpdateBase;
