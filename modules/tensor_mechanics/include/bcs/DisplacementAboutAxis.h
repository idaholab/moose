/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef DISPLACEMENTABOUTAXIS_H
#define DISPLACEMENTABOUTAXIS_H

#include "PresetNodalBC.h"

// Forward Declarations
class DisplacementAboutAxis;
class Function;

template <>
InputParameters validParams<DisplacementAboutAxis>();
void addDisplacementAboutAxisParams(InputParameters & params);

/**
 * Implements a boundary condition that enforces rotational displacement around
 * an axis on a boundary.
 */
class DisplacementAboutAxis : public PresetNodalBC
{
public:
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
  Function & _func;
  MooseEnum _angle_units;
  const Point _axis_origin;
  Point _axis_direction;

  ColumnMajorMatrix _transformation_matrix;
  ColumnMajorMatrix _transformation_matrix_inv;
};

#endif // DISPLACEMENTABOUTAXIS_H
