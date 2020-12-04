//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "INSFVMassAdvectionFunctionBC.h"

#ifdef MOOSE_GLOBAL_AD_INDEXING

#include "Function.h"

registerMooseObject("NavierStokesApp", INSFVMassAdvectionFunctionBC);

InputParameters
INSFVMassAdvectionFunctionBC::validParams()
{
  InputParameters params = INSFVMomentumAdvectionFunctionBC::validParams();
  params.addClassDescription(
      "Implements the mass equation advection term on boundaries. Only useful "
      "for MMS since it requires exact solution information.");
  params.suppressParameter<MaterialPropertyName>("advected_quantity");
  params.suppressParameter<FunctionName>("flux_variable_exact_solution");
  return params;
}

INSFVMassAdvectionFunctionBC::INSFVMassAdvectionFunctionBC(const InputParameters & params)
  : INSFVMomentumAdvectionFunctionBC(params)
{
}

ADReal
INSFVMassAdvectionFunctionBC::computeQpResidual()
{
  ADRealVectorValue v_face;

  RealVectorValue v_ghost(
      _vel_x_exact_solution.value(_t, 2. * _face_info->faceCentroid() - _face_info->elemCentroid()),
      _vel_y_exact_solution ? _vel_y_exact_solution->value(
                                  _t, 2. * _face_info->faceCentroid() - _face_info->elemCentroid())
                            : 0,
      _vel_z_exact_solution ? _vel_z_exact_solution->value(
                                  _t, 2. * _face_info->faceCentroid() - _face_info->elemCentroid())
                            : 0);

  this->interpolate(_velocity_interp_method, v_face, _vel[_qp], v_ghost);
  return _normal * v_face * _rho;
}

#endif
