//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "NonlocalKernel.h"
#include "Assembly.h"
#include "SimpleTestShapeElementUserObject.h"

#include "libmesh/quadrature.h"

class SimpleTestShapeElementKernel : public NonlocalKernel
{
public:
  static InputParameters validParams();

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
