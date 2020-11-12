//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NSFVFunctionBC.h"

#ifdef MOOSE_GLOBAL_AD_INDEXING

#include "MooseVariableFieldBase.h"
#include "SystemBase.h"
#include "ADReal.h"    // Moose::derivInsert
#include "MooseMesh.h" // FaceInfo methods
#include "FVDirichletBC.h"

#include "libmesh/dof_map.h"
#include "libmesh/elem.h"
#include "libmesh/numeric_vector.h"
#include "libmesh/vector_value.h"

registerMooseObject("NavierStokesApp", NSFVFunctionBC);

InputParameters
NSFVFunctionBC::validParams()
{
  InputParameters params = FVMatAdvectionFunctionBC::validParams();
  params += NSFVBase::validParams();

  params.addRequiredParam<FunctionName>("pressure_exact_solution",
                                        "The function describing the pressure exact solution.");
  return params;
}

NSFVFunctionBC::NSFVFunctionBC(const InputParameters & params)
  : FVMatAdvectionFunctionBC(params),
    NSFVBase(params),
    _pressure_exact_solution(getFunction("pressure_exact_solution"))
{
}

void
NSFVFunctionBC::interpolate(Moose::FV::InterpMethod m,
                            ADRealVectorValue & v_face,
                            const ADRealVectorValue & elem_v,
                            const RealVectorValue & ghost_v)
{
  Moose::FV::interpolate(Moose::FV::InterpMethod::Average, v_face, elem_v, ghost_v, *_face_info);

  if (m == Moose::FV::InterpMethod::RhieChow)
  {
    auto pr = Moose::FV::determineElemOneAndTwo(*_face_info, *_p_var);
    const Elem * const elem = pr.first;
    bool elem_is_elem = elem == &_face_info->elem();
    const Point & elem_centroid =
        elem_is_elem ? _face_info->elemCentroid() : _face_info->neighborCentroid();
    const Real elem_volume = elem_is_elem ? _face_info->elemVolume() : _face_info->neighborVolume();

    // Get pressure gradient for the elem
    const VectorValue<ADReal> & grad_p_elem = _p_var->adGradSln(elem);

    // Get pressure gradient for the ghost
    RealVectorValue grad_p_ghost =
        _pressure_exact_solution.gradient(_t, 2. * _face_info->faceCentroid() - elem_centroid);

    // Uncorrected face pressure gradient
    auto unc_grad_p = (grad_p_elem + grad_p_ghost) / 2.;

    // Now perform correction

    // Get pressure value for the elem
    ADReal p_elem = _p_var->getElemValue(elem);

    // Get pressure value for the ghost
    Real p_ghost =
        _pressure_exact_solution.value(_t, 2. * _face_info->faceCentroid() - elem_centroid);

    auto d_cf = 2. * (_face_info->faceCentroid() - elem_centroid);
    auto del_pressure = p_ghost - p_elem;

    // Direction is important. We always define things to point out from the elem
    if (!elem_is_elem)
    {
      d_cf *= -1;
      del_pressure *= -1;
    }
    const auto d_cf_norm = d_cf.norm();
    const auto e_cf = d_cf / d_cf_norm;

    // Corrected face pressure gradient
    const auto grad_p = unc_grad_p + (del_pressure / d_cf_norm - unc_grad_p * e_cf) * e_cf;

    // Now we need to perform the computations of D
    // I don't know how I would want to do computation of the a coefficient on a ghost cell. I would
    // have to essentially create an entire fictional element with defined geometric locations of
    // the faces in order to compute inward advective flux and diffusive flux. For now I'm going to
    // try not doing that and just use the a coeff of the elem
    const ADReal & face_a = rcCoeff(*elem);
    const Real face_volume = elem_volume;

    const ADReal face_D = face_volume / face_a;

    // perform the pressure correction
    v_face -= face_D * (grad_p - unc_grad_p);
  }
}

ADReal
NSFVFunctionBC::computeQpResidual()
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
  Moose::FV::interpolate(
      _advected_interp_method, flux_var_face, _adv_quant[_qp], flux_var_ghost, v_face, *_face_info);
  return _normal * v_face * flux_var_face;
}

#endif
