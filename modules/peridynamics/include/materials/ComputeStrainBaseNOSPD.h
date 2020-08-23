//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "DerivativeMaterialInterface.h"
#include "MechanicsMaterialBasePD.h"
#include "RankTwoTensor.h"

/**
 * Base material class for correspondence material model
 */
class ComputeStrainBaseNOSPD : public DerivativeMaterialInterface<MechanicsMaterialBasePD>
{
public:
  static InputParameters validParams();

  ComputeStrainBaseNOSPD(const InputParameters & parameters);
  virtual void initQpStatefulProperties() override;

protected:
  virtual void computeProperties() override;
  virtual void computeBondStretch() override;

  /**
   * Function to compute deformation gradient for peridynamic correspondence model
   */
  virtual void computeQpDeformationGradient();

  /**
   * Function to compute conventional nonlocal deformation gradient
   */
  virtual void computeConventionalQpDeformationGradient();

  /**
   * Function to compute bond-associated horizon based deformation gradient
   */
  virtual void computeBondHorizonQpDeformationGradient();

  /**
   * Function to compute strain tensors
   */
  virtual void computeQpStrain() = 0;

  /// Option of stabilization scheme for correspondence material model:
  /// FORCE, BOND_HORIZON_I or BOND_HORIZON_II
  const MooseEnum _stabilization;

  /// Plane strain problem or not, this is only used for mechanical stretch calculation
  const bool _plane_strain;

  ///@{ Material properties to fetch
  const MaterialProperty<RankFourTensor> & _Cijkl;
  std::vector<MaterialPropertyName> _eigenstrain_names;
  std::vector<const MaterialProperty<RankTwoTensor> *> _eigenstrains;
  ///@}

  ///@{ Material properties to store
  MaterialProperty<RankTwoTensor> & _shape2;
  MaterialProperty<RankTwoTensor> & _deformation_gradient;

  MaterialProperty<RankTwoTensor> & _ddgraddu;
  MaterialProperty<RankTwoTensor> & _ddgraddv;
  MaterialProperty<RankTwoTensor> & _ddgraddw;

  MaterialProperty<RankTwoTensor> & _total_strain;
  MaterialProperty<RankTwoTensor> & _mechanical_strain;

  MaterialProperty<Real> & _multi;

  /// fictitious force coefficient for force stabilized model
  MaterialProperty<Real> & _sf_coeff;
  ///@}
};
