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

#ifndef SIMPLETESTSHAPEELEMENTKERNEL_H
#define SIMPLETESTSHAPEELEMENTKERNEL_H

#include "NonlocalKernel.h"
#include "Assembly.h"
#include "SimpleTestShapeElementUserObject.h"

#include "libmesh/quadrature.h"

class SimpleTestShapeElementKernel : public NonlocalKernel
{
public:
  SimpleTestShapeElementKernel(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  /// new method for jacobian contributions corresponding to non-local dofs
  virtual Real computeQpNonlocalJacobian(dof_id_type dof_index);

  const SimpleTestShapeElementUserObject & _shp;
  const Real & _shp_integral;
  const std::vector<Real> & _shp_jacobian;

  const std::vector<dof_id_type> & _var_dofs;
};

template <>
InputParameters validParams<SimpleTestShapeElementKernel>();

#endif // SIMPLETESTSHAPEELEMENTKERNEL_H
