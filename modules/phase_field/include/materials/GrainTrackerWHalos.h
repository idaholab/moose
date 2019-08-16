//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Material.h"

// Forward Declarations
class GrainTrackerWHalos;

template <>
InputParameters validParams<GrainTrackerWHalos>();

/**
 * Calculates The Deformation Energy associated with a specific dislocation density.
 * The rest of parameters are the same as in the grain growth model
 */
class GrainTrackerWHalos : public Material
{
public:
  GrainTrackerWHalos(const InputParameters & parameters);

protected:
  virtual void computeQpProperties();

  const unsigned int _op_num;

  unsigned int _grain_num;

  /// Grain tracker object
  const GrainTrackerInterface & _grain_tracker;

  const EulerAngleProvider & _euler;

  const std::string _F_name;

  std::vector<RankTwoTensor> _orientation_matrix;
  std::vector<MaterialProperty<RankTwoTensor> *> _qp_orientation_matrix;

  std::vector<unsigned int> _op_to_grain;
};
