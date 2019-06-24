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

class NodalRankTwoPD;
class PeridynamicsMesh;

template <>
InputParameters validParams<NodalRankTwoPD>();

/**
 * Aux Kernel class to postprocess and output the strain and stress components and equivalents for
 * peridynamic models excluding correspondence material models
 */
class NodalRankTwoPD : public AuxKernelBasePD
{
public:
  NodalRankTwoPD(const InputParameters & parameters);

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
  virtual RankTwoTensor computeNodalMechanicalStrain();

  /**
   * Function to compute the stress tensor at each discrete materials point
   * @return The calculated stress tensor
   */
  virtual RankTwoTensor computeNodalStress();

  bool _has_temp;

  /// coupled temperature variable
  MooseVariable * _temp_var;

  /// bond_status variable
  MooseVariable & _bond_status_var;

  ///@{ variable for generalized plane strain cases
  bool _scalar_out_of_plane_strain_coupled;
  const VariableValue & _scalar_out_of_plane_strain;
  ///@}

  /// plane stress problem or not
  const bool _plane_stress;

  ///@{ material constants
  const Real _youngs_modulus;
  const Real _poissons_ratio;
  const Real _alpha;
  ///@}

  /// reference temperature
  const Real _temp_ref;

  /// name of rank two tensor to be processed: total_strain, mechanical_strain or stress
  std::string _rank_two_tensor;

  /// output type: component or scalar
  std::string _output_type;

  /// specific scalar type to be output
  MooseEnum _scalar_type;

  ///@{ component index
  const unsigned int _i;
  const unsigned int _j;
  ///@}

  ///@{ Points for direction dependent scalar output
  const Point _point1;
  const Point _point2;
  ///@

  /// displacement variables
  std::vector<MooseVariable *> _disp_var;

  /// elasticity tensor
  RankFourTensor _Cijkl;
};
