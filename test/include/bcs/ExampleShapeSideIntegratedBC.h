//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "NonlocalIntegratedBC.h"
#include "NumShapeSideUserObject.h"
#include "DenomShapeSideUserObject.h"

class ExampleShapeSideIntegratedBC : public NonlocalIntegratedBC
{
public:
  static InputParameters validParams();

  ExampleShapeSideIntegratedBC(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);
  /// new method for on-diagonal jacobian contributions corresponding to non-local dofs
  virtual Real computeQpNonlocalJacobian(dof_id_type dof_index);
  /// new method for off-diagonal jacobian contributions corresponding to non-local dofs
  virtual Real computeQpNonlocalOffDiagJacobian(unsigned int jvar, dof_id_type dof_index);

  const NumShapeSideUserObject & _num_shp;
  const Real & _num_shp_integral;
  const std::vector<Real> & _num_shp_jacobian;
  const DenomShapeSideUserObject & _denom_shp;
  const Real & _denom_shp_integral;
  const std::vector<Real> & _denom_shp_jacobian;

  const std::vector<dof_id_type> & _var_dofs;
  unsigned int _v_var;
  const std::vector<dof_id_type> & _v_dofs;
  Real _Vb;
};
