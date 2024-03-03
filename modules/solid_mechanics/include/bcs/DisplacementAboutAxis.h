//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "DirichletBCBase.h"

// MOOSE includes
#include "ColumnMajorMatrix.h"

// Forward Declarations
class Function;

void addDisplacementAboutAxisParams(InputParameters & params);

/**
 * Implements a boundary condition that enforces rotational displacement around
 * an axis on a boundary.
 */
class DisplacementAboutAxis : public DirichletBCBase
{
public:
  static InputParameters validParams();

  DisplacementAboutAxis(const InputParameters & parameters);

protected:
  /// Evaluate the boundary condition at the current quadrature point and timestep.
  virtual Real computeQpValue();
  virtual void initialSetup();

  /// Calculate the tranformation matrix to rotate in x, y, and z
  /// depends on the prescribed BC angle and the rotation about x and y matrices
  ColumnMajorMatrix rotateAroundAxis(const ColumnMajorMatrix & p0, const Real angle);

  /// Check if the provided axis direction vector is a unit vector and normalizes
  /// the vector if necessary during the initialization step.
  void calculateUnitDirectionVector();

  /// Calculate the rotation about the x and y axes based on the provided axis direction
  /// vector at the start of the simulation during the initialization step.
  void calculateTransformationMatrices();

  const int _component;
  const Function & _func;
  MooseEnum _angle_units;
  const Point _axis_origin;
  Point _axis_direction;

  /// number of displacement components
  const unsigned int _ndisp;

  /// the old displacement value
  std::vector<const VariableValue *> _disp_old;

  /// flag for incremental formulation
  const bool _angular_velocity;

  ColumnMajorMatrix _transformation_matrix;
  ColumnMajorMatrix _transformation_matrix_inv;
};
