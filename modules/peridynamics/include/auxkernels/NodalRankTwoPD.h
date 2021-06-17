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

class PeridynamicsMesh;

/**
 * Aux Kernel class to postprocess and output the strain and stress components and equivalents for
 * peridynamic models excluding correspondence material models
 */
class NodalRankTwoPD : public AuxKernelBasePD
{
public:
  static InputParameters validParams();

  NodalRankTwoPD(const InputParameters & parameters);

protected:
  Real computeValue() override;

  /**
   * Function to compute the total strain, mechanical strain, and stress tensors at each discrete
   * material point
   */
  virtual void computeRankTwoTensors();

  bool _has_temp;

  /// coupled temperature variable
  MooseVariable * _temp_var;

  /// bond_status variable
  MooseVariable * _bond_status_var;

  ///@{ variable for generalized plane strain cases
  bool _scalar_out_of_plane_strain_coupled;
  const VariableValue & _scalar_out_of_plane_strain;
  ///@}

  /// plane stress problem or not
  const bool _plane_stress;

  ///@{ material constants
  Real _youngs_modulus;
  Real _poissons_ratio;
  Real _alpha;
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
  unsigned int _i;
  unsigned int _j;
  ///@}

  ///@{ Points for direction dependent scalar output
  const Point _point1;
  const Point _point2;
  ///@

  /// displacement variables
  std::vector<MooseVariable *> _disp_var;

  /// elasticity tensor
  RankFourTensor _Cijkl;

  /// rank two tensors
  RankTwoTensor _total_strain;
  RankTwoTensor _mechanical_strain;
  RankTwoTensor _stress;
};
