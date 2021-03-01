//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "INSFVMomentumAdvection.h"

registerMooseObject("NavierStokesApp", INSFVMomentumAdvection);

InputParameters
INSFVMomentumAdvection::validParams()
{
  InputParameters params = INSFVAdvectionBase::validParams();

  params.addRequiredParam<Real>("rho", "The value for the density");
  params.declareControllable("rho");

  params.addClassDescription("Object for advecting momentum, e.g. rho*u");

  return params;
}

INSFVMomentumAdvection::INSFVMomentumAdvection(const InputParameters & params)
  : INSFVAdvectionBase(params), _rho(getParam<Real>("rho"))
{
}

#ifdef MOOSE_GLOBAL_AD_INDEXING
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
  //   D_e / d_{CE} * (\phi_C - \phi_E) + D_w / d_{WC} * (\phi_C - \phi_W)
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

  if (_v_var)
    elem_velocity(1) = _v_var->getElemValue(&elem);
  if (_w_var)
    elem_velocity(2) = _w_var->getElemValue(&elem);

  auto action_functor = [&coeff,
                         &elem_velocity,
                         &mu,
#ifndef NDEBUG
                         &elem,
#endif
                         this](const Elem & libmesh_dbg_var(functor_elem),
                               const Elem * const neighbor,
                               const FaceInfo * const fi,
                               const Point & surface_vector,
                               Real libmesh_dbg_var(coord),
                               const bool elem_has_info) {
    mooseAssert(fi, "We need a non-null FaceInfo");
    mooseAssert(&elem == &functor_elem, "Elems don't match");

    const Point normal = elem_has_info ? fi->normal() : Point(-fi->normal());
    const Point & rc_centroid = elem_has_info ? fi->elemCentroid() : fi->neighborCentroid();
#ifndef NDEBUG
    for (const auto i : make_range(unsigned(LIBMESH_DIM)))
      mooseAssert(
          coord == 0
              ? true
              : MooseUtils::absoluteFuzzyEqual(
                    normal(i), (surface_vector / (fi->faceArea() * coord))(i), libMesh::TOLERANCE),
          "Let's make sure our normal is what we think it is");
#endif

    // Unless specified otherwise, "elem" here refers to the element we're computing the
    // Rhie-Chow coefficient for. "neighbor" is the element across the current FaceInfo (fi)
    // face from the Rhie-Chow element

    if (onBoundary(*fi))
    {
      // Find the boundary id that has an associated INSFV boundary condition
      // if a face has more than one bc_id
      for (const auto bc_id : fi->boundaryIDs())
      {
        if (_no_slip_wall_boundaries.find(bc_id) != _no_slip_wall_boundaries.end())
        {
          // Need to account for viscous shear stress from wall
          for (const auto i : make_range(_dim))
            coeff(i) += mu * surface_vector.norm() /
                        std::abs((fi->faceCentroid() - rc_centroid) * normal) *
                        (1 - normal(i) * normal(i));

          // No flow normal to wall, so no contribution to coefficient from the advection term
          return;
        }

        if (_slip_wall_boundaries.find(bc_id) != _slip_wall_boundaries.end())
          // In the case of a slip wall we neither have viscous shear stress from the wall nor
          // normal outflow, so our contribution to the coefficient is zero
          return;

        if (_flow_boundaries.find(bc_id) != _flow_boundaries.end())
        {
          ADRealVectorValue face_velocity(_u_var->getBoundaryFaceValue(*fi));
          if (_v_var)
            face_velocity(1) = _v_var->getBoundaryFaceValue(*fi);
          if (_w_var)
            face_velocity(2) = _w_var->getBoundaryFaceValue(*fi);

          const auto advection_coeffs =
              Moose::FV::interpCoeffs(_advected_interp_method, *fi, elem_has_info, face_velocity);
          ADReal temp_coeff = _rho * face_velocity * surface_vector * advection_coeffs.first;

          if (_fully_developed_flow_boundaries.find(bc_id) ==
              _fully_developed_flow_boundaries.end())
            // We are not on a fully developed flow boundary, so we have a viscous term
            // contribution. This term is slightly modified relative to the internal face term.
            // Instead of the distance between elem and neighbor centroid, we just have the distance
            // between the elem and face centroid. Specifically, the term below is the result of
            // Moukalled 8.80, 8.82, and the orthogonal correction approach equation for E_f,
            // equation 8.89. So relative to the internal face viscous term, we have substituted
            // eqn. 8.82 for 8.78
            temp_coeff += mu * surface_vector.norm() / (fi->faceCentroid() - rc_centroid).norm();

          // For flow boundaries, the coefficient addition is the same for every velocity component
          for (const auto i : make_range(_dim))
            coeff(i) += temp_coeff;

          return;
        }

        if (_symmetry_boundaries.find(bc_id) != _symmetry_boundaries.end())
        {
          // Moukalled eqns. 15.154 - 15.156
          for (const auto i : make_range(_dim))
            coeff(i) += 2. * mu * surface_vector.norm() /
                        std::abs((fi->faceCentroid() - rc_centroid) * normal) * normal(i) *
                        normal(i);

          return;
        }
      }

      mooseError("The INSFVMomentumAdvection object ",
                 this->name(),
                 " is not completely bounded by INSFVBCs. Please examine sideset ",
                 *fi->boundaryIDs().begin(),
                 " and your FVBCs blocks.");
    }

    // Else we are on an internal face

    ADRealVectorValue neighbor_velocity(_u_var->getNeighborValue(neighbor, *fi, elem_velocity(0)));
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
                           elem_has_info);

    // we are only interested in the interpolation coefficient for the Rhie-Chow element,
    // so we just use the 'first' member of the returned pair
    const auto advection_coeffs =
        Moose::FV::interpCoeffs(_advected_interp_method, *fi, elem_has_info, interp_v);
    ADReal temp_coeff = _rho * interp_v * surface_vector * advection_coeffs.first;

    // Now add the viscous flux. Note that this includes only the orthogonal component! See
    // Moukalled equations 8.80, 8.78, and the orthogonal correction approach equation for
    // E_f, equation 8.69
    temp_coeff += mu * surface_vector.norm() / (fi->neighborCentroid() - fi->elemCentroid()).norm();

    // For internal faces the coefficient is the same for every velocity component.
    for (const auto i : make_range(_dim))
      coeff(i) += temp_coeff;
  };

  Moose::FV::loopOverElemFaceInfo(elem, _subproblem.mesh(), _subproblem, action_functor);

  return coeff;
}

#else

VectorValue<ADReal>
INSFVMomentumAdvection::coeffCalculator(const Elem &, const ADReal &) const
{
  mooseError("INSFVMomentumAdvection only works with global AD indexing");
}

#endif
