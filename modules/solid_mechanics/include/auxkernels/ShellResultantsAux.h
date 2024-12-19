//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "AuxKernel.h"
#include "RankTwoTensor.h"
#include "MooseEnum.h"

// Forward declarations
namespace libMesh
{
class QGauss;
}

class ShellResultantsAux : public AuxKernel
{
public:
  static InputParameters validParams();

  ShellResultantsAux(const InputParameters & parameters);

protected:
  virtual Real computeValue();

  /// Base name of the material system used to calculate the elastic energy
  const std::string _base_name;

  /// Coupled variable for the shell thickness
  const VariableValue & _thickness;

  enum class ResultantType
  {
    axial_force_0,
    axial_force_1,
    normal_force,
    bending_moment_0,
    bending_moment_1,
    bending_moment_01,
    shear_force_01,
    shear_force_02,
    shear_force_12
  } _resultant;
  /// Quadrature rule in the out of plane direction
  std::unique_ptr<QGauss> _t_qrule;

  /// Quadrature points and weights in the out of plane direction in isoparametric coordinate system
  std::vector<Point> _t_points;
  std::vector<Real> _t_weights;

  /// The local stress tensor
  std::vector<const MaterialProperty<RankTwoTensor> *> _local_stress_t_points;
};
