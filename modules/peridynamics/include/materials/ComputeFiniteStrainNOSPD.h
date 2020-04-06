//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ComputeStrainBaseNOSPD.h"
#include "RankTwoTensor.h"

/**
 * Material class for peridynamic correspondence model for finite strain
 */
class ComputeFiniteStrainNOSPD : public ComputeStrainBaseNOSPD
{
public:
  static InputParameters validParams();

  ComputeFiniteStrainNOSPD(const InputParameters & parameters);

  static MooseEnum decompositionType();

protected:
  virtual void computeQpStrain() override;
  virtual void computeQpFhat();

  /**
   * Function to compute strain and rotational increments
   */
  void computeQpStrainRotationIncrements(RankTwoTensor & e, RankTwoTensor & r);

  /**
   * Function to compute the mechanical strain tensor by subtracting thermal strain from the total
   * strain
   */
  void subtractEigenstrainIncrementFromStrain(RankTwoTensor & strain);

  ///@{ Material properties to store
  MaterialProperty<RankTwoTensor> & _strain_rate;
  MaterialProperty<RankTwoTensor> & _strain_increment;
  MaterialProperty<RankTwoTensor> & _rotation_increment;
  ///@}

  ///@{ Material properties to fetch
  const MaterialProperty<RankTwoTensor> & _deformation_gradient_old;
  const MaterialProperty<RankTwoTensor> & _mechanical_strain_old;
  const MaterialProperty<RankTwoTensor> & _total_strain_old;
  std::vector<const MaterialProperty<RankTwoTensor> *> _eigenstrains_old;
  ///@}

  /// 'Incremental' deformation gradient
  std::vector<RankTwoTensor> _Fhat;

private:
  ///@{ Method to decompose into rotation increment and strain increment
  enum class DecompMethod
  {
    TaylorExpansion,
    EigenSolution
  };
  const DecompMethod _decomposition_method;
  ///@}
};
