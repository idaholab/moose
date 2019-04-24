//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "AuxKernelBasePD.h"
#include "RankTwoTensor.h"
#include "RankFourTensor.h"

class NodalStressStrainPD;
class PeridynamicsMesh;

template <>
InputParameters validParams<NodalStressStrainPD>();

/**
 * Aux Kernel class to postprocess and output the strain and stress components and equivalents for
 * peridynamic models excluding correspondence material models
 */
class NodalStressStrainPD : public AuxKernelBasePD
{
public:
  NodalStressStrainPD(const InputParameters & parameters);

protected:
  Real computeValue() override;

  /**
   * Function to compute the total strain tensor at each discrete material point
   * @return The calculated total strain tensor
   */
  virtual RankTwoTensor computeNodalTotalStrain();

  /**
   * Function to compute the elastic strain tensor at each discrete material point
   * @return The calculated elastic strain tensor
   */
  virtual RankTwoTensor computeNodalElasticStrain();

  /**
   * Function to compute the stress tensor at each discrete materials point
   * @return The calculated stress tensor
   */
  virtual RankTwoTensor computeNodalStress();

  /// coupled temperature variable
  MooseVariableFEBase * _temp_var;

  /// bond_status variable
  MooseVariableFEBase & _bond_status_var;

  ///@{ variable for generalized plane strain cases
  bool _scalar_out_of_plane_strain_coupled;
  const VariableValue & _scalar_out_of_plane_strain;
  ///@}

  /// plane stress problem or not
  const bool _plane_stress;

  ///@{ material constants
  const Real _youngs_modulus;
  const Real _poissons_ratio;
  Real _alpha;
  ///@}

  /// reference temperature
  const Real _temp_ref;

  /// name of rank two tensor to be processed: total_strain, elastic_strain or stress
  std::string _rank_two_tensor;

  /// output type: components of strain/stress tensor, or vonMisesStress
  MooseEnum _quantity_type;

  ///@{ component index
  unsigned int _i;
  unsigned int _j;
  ///@}

  /// displacement variables
  std::vector<MooseVariableFEBase *> _disp_var;

  /// Rank four elasticity tensor
  RankFourTensor _Cijkl;
};
