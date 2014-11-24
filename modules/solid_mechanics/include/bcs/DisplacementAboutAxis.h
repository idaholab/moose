/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#ifndef DISPLACEMENTABOUTAXIS_H
#define DISPLACEMENTABOUTAXIS_H

#include "PresetNodalBC.h"

//Forward Declarations
class DisplacementAboutAxis;
class Function;

template<>
InputParameters validParams<DisplacementAboutAxis>();
void addDisplacementAboutAxisParams(InputParameters& params);

/**
 * Implements a boundary condition that enforces rotational displacement around
 * an axis on a boundary.
 */
class DisplacementAboutAxis : public PresetNodalBC
{
public:
  DisplacementAboutAxis(const std::string & name, InputParameters parameters);

protected:
  /**
   * Evaluate the function at the current quadrature point and timestep.
   */
  virtual Real computeQpValue();
  virtual void initialSetup();
  ColumnMajorMatrix rotateAroundAxis(const ColumnMajorMatrix & p0, const Real angle);
  void calculateTransformationMatrices();

  const int _component;
  Function & _func;
  MooseEnum _angle_units;
  const Point _axis_origin;
  const Point _axis_direction;

  ColumnMajorMatrix _transformation_matrix;
  ColumnMajorMatrix _transformation_matrix_inv;
};

#endif //DISPLACEMENTABOUTAXIS_H
