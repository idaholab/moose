//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "INSFVMomentumAdvectionFunctionBC.h"

#ifdef MOOSE_GLOBAL_AD_INDEXING

#include "MooseVariableFieldBase.h"
#include "SystemBase.h"
#include "ADReal.h"    // Moose::derivInsert
#include "MooseMesh.h" // FaceInfo methods
#include "FVDirichletBC.h"
#include "Function.h"

#include "libmesh/dof_map.h"
#include "libmesh/elem.h"
#include "libmesh/numeric_vector.h"
#include "libmesh/vector_value.h"

registerMooseObject("NavierStokesApp", INSFVMomentumAdvectionFunctionBC);

InputParameters
INSFVMomentumAdvectionFunctionBC::validParams()
{
  InputParameters params = FVMatAdvectionFunctionBC::validParams();
  params.addClassDescription(
      "Implements the momentum equation advection term on boundaries. Only useful "
      "for MMS since it requires exact solution information.");
  params += INSFVAdvectionBase::validParams();

  // We need 2 ghost layers for the Rhie-Chow interpolation
  params.set<unsigned short>("ghost_layers") = 2;

  params.addRequiredParam<FunctionName>("pressure_exact_solution",
                                        "The function describing the pressure exact solution.");
  return params;
}

INSFVMomentumAdvectionFunctionBC::INSFVMomentumAdvectionFunctionBC(const InputParameters & params)
  : FVMatAdvectionFunctionBC(params),
    INSFVAdvectionBase(params),
    _pressure_exact_solution(getFunction("pressure_exact_solution")),
    _mu(getADMaterialProperty<Real>("mu"))
{
}

void
INSFVMomentumAdvectionFunctionBC::interpolate(Moose::FV::InterpMethod m,
                                              ADRealVectorValue & v_face,
                                              const ADRealVectorValue & elem_v,
                                              const RealVectorValue & ghost_v)
{
  const auto tup = Moose::FV::determineElemOneAndTwo(*_face_info, *_p_var);
  const Elem * const elem = std::get<0>(tup);
  const bool elem_is_elem = std::get<2>(tup);
  mooseAssert(elem_is_elem ? elem == &_face_info->elem() : elem == _face_info->neighborPtr(),
              "elem_is_elem is incorrect");

  Moose::FV::interpolate(
      Moose::FV::InterpMethod::Average, v_face, elem_v, ghost_v, *_face_info, elem_is_elem);

  if (m != Moose::FV::InterpMethod::RhieChow)
    return;

  const auto & elem_centroid =
      elem_is_elem ? _face_info->elemCentroid() : _face_info->neighborCentroid();
  const auto elem_volume = elem_is_elem ? _face_info->elemVolume() : _face_info->neighborVolume();

  // Get pressure gradient for the elem
  const auto & grad_p_elem = _p_var->adGradSln(elem);

  // Get pressure gradient for the ghost
  const auto & grad_p_ghost =
      _pressure_exact_solution.gradient(_t, 2. * _face_info->faceCentroid() - elem_centroid);

  // Uncorrected face pressure gradient. Geometric weights are exactly 1/2 because we arbitrarily
  // choose the ghost element centroid such that the face centroid is exactly in the middle of the
  // line connecting element and ghost element centroids (see code line above)
  const auto unc_grad_p = (grad_p_elem + grad_p_ghost) / 2.;

  // Now perform correction

  // Get pressure value for the elem
  const auto & p_elem = _p_var->getElemValue(elem);

  // Get pressure value for the ghost
  const auto & p_ghost =
      _pressure_exact_solution.value(_t, 2. * _face_info->faceCentroid() - elem_centroid);

  const auto d_cf = 2. * (_face_info->faceCentroid() - elem_centroid);
  const auto del_pressure = p_ghost - p_elem;

  const auto d_cf_norm = d_cf.norm();
  const auto e_cf = d_cf / d_cf_norm;

  // Corrected face pressure gradient
  const auto grad_p = unc_grad_p + (del_pressure / d_cf_norm - unc_grad_p * e_cf) * e_cf;

  // Moukalled 15.110 suggests that is valid to use the element centroid value as the average value
  // on the boundary face
  const auto & face_a = rcCoeff(*elem, _mu[_qp]);
  const auto & face_volume = elem_volume;

  const auto face_D = face_volume / face_a;

  // perform the pressure correction
  v_face -= face_D * (grad_p - unc_grad_p);
}

ADReal
INSFVMomentumAdvectionFunctionBC::computeQpResidual()
{
  ADReal flux_var_face;
  ADRealVectorValue v_face;

  Real flux_var_ghost = _flux_variable_exact_solution.value(
      _t, 2. * _face_info->faceCentroid() - _face_info->elemCentroid());
  RealVectorValue v_ghost(
      _vel_x_exact_solution.value(_t, 2. * _face_info->faceCentroid() - _face_info->elemCentroid()),
      _vel_y_exact_solution ? _vel_y_exact_solution->value(
                                  _t, 2. * _face_info->faceCentroid() - _face_info->elemCentroid())
                            : 0,
      _vel_z_exact_solution ? _vel_z_exact_solution->value(
                                  _t, 2. * _face_info->faceCentroid() - _face_info->elemCentroid())
                            : 0);

  this->interpolate(_velocity_interp_method, v_face, _vel[_qp], v_ghost);
  Moose::FV::interpolate(_advected_interp_method,
                         flux_var_face,
                         _adv_quant[_qp],
                         flux_var_ghost,
                         v_face,
                         *_face_info,
                         true);
  return _normal * v_face * flux_var_face;
}

#endif
