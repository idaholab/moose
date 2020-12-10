//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "INSFVMomentumAdvection.h"

#include "MooseVariableFieldBase.h"
#include "SystemBase.h"
#include "ADReal.h"    // Moose::derivInsert
#include "MooseMesh.h" // FaceInfo methods
#include "FVDirichletBC.h"

#include "libmesh/dof_map.h"
#include "libmesh/elem.h"
#include "libmesh/numeric_vector.h"
#include "libmesh/vector_value.h"

registerMooseObject("NavierStokesApp", INSFVMomentumAdvection);

InputParameters
INSFVMomentumAdvection::validParams()
{
  InputParameters params = FVMatAdvection::validParams();
  params += INSFVAdvectionBase::validParams();

  // We need 2 ghost layers for the Rhie-Chow interpolation
  params.set<unsigned short>("ghost_layers") = 2;

  params.addClassDescription("Object for advecting momentum, e.g. rho*u");

  return params;
}

INSFVMomentumAdvection::INSFVMomentumAdvection(const InputParameters & params)
  : FVMatAdvection(params),
    INSFVAdvectionBase(params),
    _mu_elem(getADMaterialProperty<Real>("mu")),
    _mu_neighbor(getNeighborADMaterialProperty<Real>("mu"))
{
#ifndef MOOSE_GLOBAL_AD_INDEXING
  mooseError("INSFV is not supported by local AD indexing. In order to use INSFV, please run the "
             "configure script in the root MOOSE directory with the configure option "
             "'--with-ad-indexing-type=global'");
#endif
}

#ifdef MOOSE_GLOBAL_AD_INDEXING
const VectorValue<ADReal> &
INSFVMomentumAdvection::rcCoeff(const Elem & elem, const ADReal & mu) const
{
  auto it = _rc_a_coeffs.find(&_nsfv_app);
  mooseAssert(it != _rc_a_coeffs.end(),
              "No RC coeffs structure exists for the given MooseApp pointer");
  mooseAssert(_nsfv_tid < it->second.size(),
              "The RC coeffs structure size "
                  << it->second.size() << " is greater than or equal to the provided thread ID "
                  << _nsfv_tid);
  auto & my_map = it->second[_nsfv_tid];

  auto rc_map_it = my_map.find(&elem);

  if (rc_map_it != my_map.end())
    return rc_map_it->second;

  // Returns a pair with the first being an iterator pointing to the key-value pair and the second a
  // boolean denoting whether a new insertion took place
  auto emplace_ret = my_map.emplace(&elem, coeffCalculator(elem, mu));

  mooseAssert(emplace_ret.second, "We should have inserted a new key-value pair");

  return emplace_ret.first->second;
}

VectorValue<ADReal>
INSFVMomentumAdvection::coeffCalculator(const Elem & elem, const ADReal & mu) const
{
  // these coefficients arise from simple control volume balances of advection and diffusion. These
  // coefficients are the linear coefficients associated with the centroid of the control volume.
  // Note that diffusion coefficients should always be positive, e.g. elliptic operators always
  // yield positive definite matrices
  //
  // Example 1D discretization of diffusion, e.g. the sum of the fluxes around a control volume:
  //
  // \sum_f -D \nabla \phi * \hat{n} =
  //   -D_e * (phi_E - \phi_C) / d_{CE} * 1 - D_w * (\phi_C - \phi_W) / d_{WC} * -1 =
  //   D_w / d_{CE} * (\phi_C - \phi_E) + D_e / d_{WC} * (\phi_C - \phi_W)
  //
  // Note the positive coefficients for \phi_C !!
  //
  // Now an example 1D discretization for advection using central differences, e.g. an average
  // interpolation
  //
  // \sum_f \vec{u} \phi \hat{n} =
  //   u_w * (\phi_W + \phi_C) / 2 * -1 + u_e * (\phi_C + \phi_E) / 2 * 1 =
  //   -u_w / 2 * \phi_W + u_e / 2 * \phi_E + (u_e - u_w) / 2 * \phi_C
  //
  // Note that the coefficient for \phi_C may or may not be positive depending on the values of u_e
  // and u_w

  VectorValue<ADReal> coeff = 0;

  ADRealVectorValue elem_velocity(_u_var->getElemValue(&elem));
  unsigned int dim = 1;

  if (_v_var)
  {
    elem_velocity(1) = _v_var->getElemValue(&elem);
    ++dim;
  }
  if (_w_var)
  {
    elem_velocity(2) = _w_var->getElemValue(&elem);
    ++dim;
  }

  auto action_functor = [&coeff, &elem_velocity, &mu, &dim, this](const Elem & /*functor_elem*/,
                                                                  const Elem * const neighbor,
                                                                  const FaceInfo * const fi,
                                                                  const Point & surface_vector,
                                                                  Real /*coord*/,
                                                                  const bool /*elem_has_info*/) {
    mooseAssert(fi, "We need a non-null FaceInfo");

    if (fi->isBoundary())
    {
      for (const auto i : make_range(dim))
        coeff(i) += mu * surface_vector.norm() / (fi->faceCentroid() - fi->elemCentroid()).norm() *
                    (1 - fi->normal()(i) * fi->normal()(i));
    }
    else
    {
      ADRealVectorValue neighbor_velocity(
          _u_var->getNeighborValue(neighbor, *fi, elem_velocity(0)));
      if (_v_var)
        neighbor_velocity(1) = _v_var->getNeighborValue(neighbor, *fi, elem_velocity(1));
      if (_w_var)
        neighbor_velocity(2) = _w_var->getNeighborValue(neighbor, *fi, elem_velocity(2));

      ADRealVectorValue interp_v;
      Moose::FV::interpolate(Moose::FV::InterpMethod::Average,
                             interp_v,
                             elem_velocity,
                             neighbor_velocity,
                             *fi,
                             neighbor == fi->neighborPtr());

      // For internal faces the coefficient is the same for every velocity component. Here we assume
      // an average interpolation
      ADReal temp_coeff = _rho * interp_v * surface_vector / 2.;

      // Now add the viscous flux. Note that this includes only the orthogonal component! See
      // Moukalled equations 8.80, 8.78, and the orthogonal correction approach equation for E_f,
      // eqaution 8.69
      temp_coeff +=
          mu * surface_vector.norm() / (fi->neighborCentroid() - fi->elemCentroid()).norm();

      for (const auto i : make_range(dim))
        coeff(i) += temp_coeff;
    }
  };

  Moose::FV::loopOverElemFaceInfo(elem, _nsfv_subproblem.mesh(), _nsfv_subproblem, action_functor);

  return coeff;
}

void
INSFVMomentumAdvection::interpolate(Moose::FV::InterpMethod m,
                                    ADRealVectorValue & v,
                                    const ADRealVectorValue & elem_v,
                                    const ADRealVectorValue & neighbor_v)
{
  auto tup = Moose::FV::determineElemOneAndTwo(*_face_info, *_p_var);
  const Elem * const elem_one = std::get<0>(tup);
  const Elem * const elem_two = std::get<1>(tup);
  const bool elem_is_elem_one = std::get<2>(tup);
  mooseAssert(elem_is_elem_one
                  ? elem_one == &_face_info->elem() && elem_two == _face_info->neighborPtr()
                  : elem_one == _face_info->neighborPtr() && elem_two == &_face_info->elem(),
              "The determineElemOneAndTwo utility determined the wrong value for elem_is_elem_one");

  if (!_face_info->isBoundary() || _velocity_interp_method == Moose::FV::InterpMethod::Average)
    Moose::FV::interpolate(
        Moose::FV::InterpMethod::Average, v, elem_v, neighbor_v, *_face_info, true);
  else
  {
    mooseAssert(_velocity_interp_method == Moose::FV::InterpMethod::RhieChow,
                "I don't know another velocity interpolation method at this time");
    mooseAssert(_face_info->isBoundary(), "We should be along a boundary");
    v = elem_is_elem_one ? elem_v : neighbor_v;
  }

  if (m != Moose::FV::InterpMethod::RhieChow)
    return;

  // Get pressure gradient. This is the uncorrected gradient plus a correction from cell centroid
  // values on either side of the face
  const VectorValue<ADReal> & grad_p = _p_var->adGradSln(*_face_info);

  // Get uncorrected pressure gradient. This will use the element centroid gradient if we are along
  // a boundary face
  const VectorValue<ADReal> & unc_grad_p = _p_var->uncorrectedAdGradSln(*_face_info);

  const Point & elem_one_centroid =
      elem_is_elem_one ? _face_info->elemCentroid() : _face_info->neighborCentroid();
  const Point * const elem_two_centroid =
      elem_two ? (elem_is_elem_one ? &_face_info->neighborCentroid() : &_face_info->elemCentroid())
               : nullptr;
  Real elem_one_volume = elem_is_elem_one ? _face_info->elemVolume() : _face_info->neighborVolume();
  Real elem_two_volume =
      elem_two ? (elem_is_elem_one ? _face_info->neighborVolume() : _face_info->elemVolume()) : 0;

  const auto & elem_one_mu = elem_is_elem_one ? _mu_elem[_qp] : _mu_neighbor[_qp];

  // Now we need to perform the computations of D
  const VectorValue<ADReal> & elem_one_a = rcCoeff(*elem_one, elem_one_mu);

  mooseAssert(elem_two ? _subproblem.getCoordSystem(elem_one->subdomain_id()) ==
                             _subproblem.getCoordSystem(elem_two->subdomain_id())
                       : true,
              "Coordinate systems must be the same between the two elements");

  Real coord;
  coordTransformFactor(_subproblem, elem_one->subdomain_id(), elem_one_centroid, coord);

  elem_one_volume *= coord;

  unsigned int dim = 1;
  if (_v_var)
    ++dim;
  if (_w_var)
    ++dim;

  VectorValue<ADReal> elem_one_D = 0;
  for (const auto i : make_range(dim))
  {
    mooseAssert(elem_one_a(i).value() != 0, "We should not be dividing by zero");
    elem_one_D(i) = elem_one_volume / elem_one_a(i);
  }

  VectorValue<ADReal> face_D;

  if (elem_two && this->hasBlocks(elem_two->subdomain_id()))
  {
    const auto & elem_two_mu = elem_is_elem_one ? _mu_neighbor[_qp] : _mu_elem[_qp];

    const VectorValue<ADReal> & elem_two_a = rcCoeff(*elem_two, elem_two_mu);

    coordTransformFactor(_subproblem, elem_two->subdomain_id(), *elem_two_centroid, coord);
    elem_two_volume *= coord;

    VectorValue<ADReal> elem_two_D = 0;
    for (const auto i : make_range(dim))
    {
      mooseAssert(elem_two_a(i).value() != 0, "We should not be dividing by zero");
      elem_two_D(i) = elem_two_volume / elem_two_a(i);
    }
    Moose::FV::interpolate(Moose::FV::InterpMethod::Average,
                           face_D,
                           elem_one_D,
                           elem_two_D,
                           *_face_info,
                           elem_is_elem_one);
  }
  else
    face_D = elem_one_D;

  // perform the pressure correction
  for (const auto i : make_range(dim))
    v(i) -= face_D(i) * (grad_p(i) - unc_grad_p(i));
}
#else
void
INSFVMomentumAdvection::interpolate(Moose::FV::InterpMethod,
                                    ADRealVectorValue &,
                                    const ADRealVectorValue &,
                                    const ADRealVectorValue &)
{
}
#endif

ADReal
INSFVMomentumAdvection::computeQpResidual()
{
  ADRealVectorValue v;
  ADReal u_interface;

  this->interpolate(_velocity_interp_method, v, _vel_elem[_qp], _vel_neighbor[_qp]);
  Moose::FV::interpolate(_advected_interp_method,
                         u_interface,
                         _adv_quant_elem[_qp],
                         _adv_quant_neighbor[_qp],
                         v,
                         *_face_info,
                         true);
  return _normal * v * u_interface;
}
