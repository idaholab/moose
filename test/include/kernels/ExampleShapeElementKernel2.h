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
#include "ExampleShapeElementUserObject.h"

class ExampleShapeElementKernel2 : public NonlocalKernel
{
public:
  static InputParameters validParams();

  ExampleShapeElementKernel2(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual() override;
  virtual Real computeQpOffDiagJacobian(unsigned int jvar) override;
  /// new method for off-diagonal jacobian contributions corresponding to non-local dofs
  virtual Real computeQpNonlocalOffDiagJacobian(unsigned int jvar, dof_id_type dof_index) override;

  const ExampleShapeElementUserObject & _shp;
  const Real & _shp_integral;
  const std::vector<Real> & _shp_jacobian;

  unsigned int _u_var;
  const std::vector<dof_id_type> & _u_dofs;
  unsigned int _v_var;
  const std::vector<dof_id_type> & _v_dofs;
};
