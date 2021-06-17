//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "INSFVMomentumAdvection.h"

#include "INSFVPressureVariable.h"
#include "INSFVVelocityVariable.h"
#include "SystemBase.h"
#include "MooseMesh.h"
#include "FVDirichletBC.h"
#include "INSFVFlowBC.h"
#include "INSFVFullyDevelopedFlowBC.h"
#include "INSFVNoSlipWallBC.h"
#include "INSFVSlipWallBC.h"
#include "INSFVSymmetryBC.h"
#include "INSFVAttributes.h"
#include "MooseUtils.h"

#include "libmesh/dof_map.h"
#include "libmesh/elem.h"
#include "libmesh/numeric_vector.h"
#include "libmesh/vector_value.h"

#include <algorithm>

registerMooseObject("NavierStokesApp", INSFVMomentumAdvection);

std::unordered_map<const MooseApp *,
                   std::vector<std::unordered_map<const Elem *, VectorValue<ADReal>>>>
    INSFVMomentumAdvection::_rc_a_coeffs;

InputParameters
INSFVMomentumAdvection::validParams()
{
  InputParameters params = FVMatAdvection::validParams();
  params.addRequiredCoupledVar("pressure", "The pressure variable.");
  params.addRequiredCoupledVar("u", "The velocity in the x direction.");
  params.addCoupledVar("v", "The velocity in the y direction.");
  params.addCoupledVar("w", "The velocity in the z direction.");

  MooseEnum velocity_interp_method("average rc", "rc");

  params.addParam<MooseEnum>(
      "velocity_interp_method",
      velocity_interp_method,
      "The interpolation to use for the velocity. Options are "
      "'average' and 'rc' which stands for Rhie-Chow. The default is Rhie-Chow.");

  params.addRequiredParam<MaterialPropertyName>("mu", "The viscosity");
  params.addRequiredParam<Real>("rho", "The value for the density");
  params.declareControllable("rho");

  // We need 2 ghost layers for the Rhie-Chow interpolation
  params.set<unsigned short>("ghost_layers") = 2;

  params.addClassDescription("Object for advecting momentum, e.g. rho*u");

  return params;
}

INSFVMomentumAdvection::INSFVMomentumAdvection(const InputParameters & params)
  : FVMatAdvection(params),
    _mu_elem(getADMaterialProperty<Real>("mu")),
    _mu_neighbor(getNeighborADMaterialProperty<Real>("mu")),
    _p_var(dynamic_cast<const INSFVPressureVariable *>(getFieldVar("pressure", 0))),
    _u_var(dynamic_cast<const INSFVVelocityVariable *>(getFieldVar("u", 0))),
    _v_var(params.isParamValid("v")
               ? dynamic_cast<const INSFVVelocityVariable *>(getFieldVar("v", 0))
               : nullptr),
    _w_var(params.isParamValid("w")
               ? dynamic_cast<const INSFVVelocityVariable *>(getFieldVar("w", 0))
               : nullptr),
    _rho(getParam<Real>("rho")),
    _dim(_subproblem.mesh().dimension())
{
#ifndef MOOSE_GLOBAL_AD_INDEXING
  mooseError("INSFV is not supported by local AD indexing. In order to use INSFV, please run the "
             "configure script in the root MOOSE directory with the configure option "
             "'--with-ad-indexing-type=global'");
#endif

  if (!_p_var)
    paramError("pressure", "the pressure must be a INSFVPressureVariable.");

  if (!_u_var)
    paramError("u", "the u velocity must be an INSFVVelocityVariable.");

  if (_dim >= 2 && !_v_var)
    paramError("v",
               "In two or more dimensions, the v velocity must be supplied and it must be an "
               "INSFVVelocityVariable.");

  if (_dim >= 3 && !params.isParamValid("w"))
    paramError("w",
               "In three-dimensions, the w velocity must be supplied and it must be an "
               "INSFVVelocityVariable.");

  const auto & velocity_interp_method = params.get<MooseEnum>("velocity_interp_method");
  if (velocity_interp_method == "average")
    _velocity_interp_method = Moose::FV::InterpMethod::Average;
  else if (velocity_interp_method == "rc")
    _velocity_interp_method = Moose::FV::InterpMethod::RhieChow;
  else
    mooseError("Unrecognized interpolation type ",
               static_cast<std::string>(velocity_interp_method));

  if (_tid == 0)
  {
    auto & vec_of_coeffs_map = _rc_a_coeffs[&_app];
    vec_of_coeffs_map.resize(libMesh::n_threads());
  }

  if (getParam<bool>("force_boundary_execution"))
    paramError("force_boundary_execution",
               "Do not use the force_boundary_execution parameter to control execution of INSFV "
               "advection objects");

  if (!getParam<std::vector<BoundaryName>>("boundaries_to_force").empty())
    paramError("boundaries_to_force",
               "Do not use the boundaries_to_force parameter to control execution of INSFV "
               "advection objects");
}

void
INSFVMomentumAdvection::initialSetup()
{
  std::set<BoundaryID> all_connected_boundaries;
  const auto & blk_ids = blockRestricted() ? blockIDs() : _mesh.meshSubdomains();
  for (const auto blk_id : blk_ids)
  {
    const auto & connected_boundaries = _mesh.getSubdomainBoundaryIds(blk_id);
    for (const auto bnd_id : connected_boundaries)
      all_connected_boundaries.insert(bnd_id);
  }

  for (const auto bnd_id : all_connected_boundaries)
  {
    setupFlowBoundaries(bnd_id);
    setupBoundaries<INSFVNoSlipWallBC>(
        bnd_id, INSFVBCs::INSFVNoSlipWallBC, _no_slip_wall_boundaries);
    setupBoundaries<INSFVSlipWallBC>(bnd_id, INSFVBCs::INSFVSlipWallBC, _slip_wall_boundaries);
    setupBoundaries<INSFVSymmetryBC>(bnd_id, INSFVBCs::INSFVSymmetryBC, _symmetry_boundaries);
  }
}

void
INSFVMomentumAdvection::setupFlowBoundaries(const BoundaryID bnd_id)
{
  std::vector<INSFVFlowBC *> flow_bcs;

  this->_subproblem.getMooseApp()
      .theWarehouse()
      .query()
      .template condition<AttribBoundaries>(bnd_id)
      .template condition<AttribINSFVBCs>(INSFVBCs::INSFVFlowBC)
      .queryInto(flow_bcs);

  if (!flow_bcs.empty())
  {
    if (dynamic_cast<INSFVFullyDevelopedFlowBC *>(flow_bcs.front()))
    {
      _fully_developed_flow_boundaries.insert(bnd_id);

#ifndef NDEBUG
      for (auto * flow_bc : flow_bcs)
        mooseAssert(dynamic_cast<INSFVFullyDevelopedFlowBC *>(flow_bc),
                    "If one BC is a fully developed flow BC, then all other flow BCs on that "
                    "boundary must also be fully developed flow BCs");
    }
    else
      for (auto * flow_bc : flow_bcs)
        mooseAssert(!dynamic_cast<INSFVFullyDevelopedFlowBC *>(flow_bc),
                    "If one BC is not a fully developed flow BC, then all other flow BCs on that "
                    "boundary must also not be fully developed flow BCs");
#else
    }
#endif

    _flow_boundaries.insert(bnd_id);
    _all_boundaries.insert(bnd_id);
  }
}

template <typename T>
void
INSFVMomentumAdvection::setupBoundaries(const BoundaryID bnd_id,
                                        const INSFVBCs bc_type,
                                        std::set<BoundaryID> & bnd_ids)
{
  std::vector<T *> bcs;

  this->_subproblem.getMooseApp()
      .theWarehouse()
      .query()
      .template condition<AttribBoundaries>(bnd_id)
      .template condition<AttribINSFVBCs>(bc_type)
      .queryInto(bcs);

  if (!bcs.empty())
  {
    bnd_ids.insert(bnd_id);
    _all_boundaries.insert(bnd_id);
  }
}

bool
INSFVMomentumAdvection::skipForBoundary(const FaceInfo & fi) const
{
  if (!onBoundary(fi))
    return false;

  // If we have flux bcs then we do skip
  const auto & flux_pr = _var.getFluxBCs(fi);
  if (flux_pr.first)
    return true;

  // If we have a flow boundary without a replacement flux BC, then we must not skip. Mass and
  // momentum are transported via advection across boundaries
  for (const auto bc_id : fi.boundaryIDs())
    if (_flow_boundaries.find(bc_id) != _flow_boundaries.end())
      return false;

  // If not a flow boundary, then there should be no advection/flow in the normal direction, e.g. we
  // should not contribute any advective flux
  return true;
}

const VectorValue<ADReal> &
INSFVMomentumAdvection::rcCoeff(const Elem & elem, const ADReal & mu) const
{
  auto it = _rc_a_coeffs.find(&_app);
  mooseAssert(it != _rc_a_coeffs.end(),
              "No RC coeffs structure exists for the given MooseApp pointer");
  mooseAssert(_tid < it->second.size(),
              "The RC coeffs structure size "
                  << it->second.size() << " is greater than or equal to the provided thread ID "
                  << _tid);
  auto & my_map = it->second[_tid];

  auto rc_map_it = my_map.find(&elem);

  if (rc_map_it != my_map.end())
    return rc_map_it->second;

  // Returns a pair with the first being an iterator pointing to the key-value pair and the second a
  // boolean denoting whether a new insertion took place
  auto emplace_ret = my_map.emplace(&elem, coeffCalculator(elem, mu));

  mooseAssert(emplace_ret.second, "We should have inserted a new key-value pair");

  return emplace_ret.first->second;
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

void
INSFVMomentumAdvection::interpolate(Moose::FV::InterpMethod m,
                                    ADRealVectorValue & v,
                                    const ADRealVectorValue & elem_v,
                                    const ADRealVectorValue & neighbor_v)
{
  const Elem * const elem = &_face_info->elem();
  const Elem * const neighbor = _face_info->neighborPtr();

  if (onBoundary(*_face_info))
  {
#ifndef NDEBUG
    bool flow_boundary_found = false;
    for (const auto b_id : _face_info->boundaryIDs())
      if (_flow_boundaries.find(b_id) != _flow_boundaries.end())
      {
        flow_boundary_found = true;
        break;
      }

    mooseAssert(flow_boundary_found,
                "INSFV*Advection flux kernel objects should only execute on flow boundaries.");
#endif

    v(0) = _u_var->getBoundaryFaceValue(*_face_info);
    if (_v_var)
      v(1) = _v_var->getBoundaryFaceValue(*_face_info);
    if (_w_var)
      v(2) = _w_var->getBoundaryFaceValue(*_face_info);

    return;
  }

  Moose::FV::interpolate(
      Moose::FV::InterpMethod::Average, v, elem_v, neighbor_v, *_face_info, true);

  if (m == Moose::FV::InterpMethod::Average)
    return;

  // Get pressure gradient. This is the uncorrected gradient plus a correction from cell centroid
  // values on either side of the face
  const VectorValue<ADReal> & grad_p = _p_var->adGradSln(*_face_info);

  // Get uncorrected pressure gradient. This will use the element centroid gradient if we are
  // along a boundary face
  const VectorValue<ADReal> & unc_grad_p = _p_var->uncorrectedAdGradSln(*_face_info);

  const Point & elem_centroid = _face_info->elemCentroid();
  const Point * const neighbor_centroid = neighbor ? &_face_info->neighborCentroid() : nullptr;
  Real elem_volume = _face_info->elemVolume();
  Real neighbor_volume = neighbor ? _face_info->neighborVolume() : 0;
  const auto & elem_mu = _mu_elem[_qp];

  // Now we need to perform the computations of D
  const VectorValue<ADReal> & elem_a = rcCoeff(*elem, elem_mu);

  mooseAssert(neighbor ? _subproblem.getCoordSystem(elem->subdomain_id()) ==
                             _subproblem.getCoordSystem(neighbor->subdomain_id())
                       : true,
              "Coordinate systems must be the same between the two elements");

  Real coord;
  coordTransformFactor(_subproblem, elem->subdomain_id(), elem_centroid, coord);

  elem_volume *= coord;

  VectorValue<ADReal> elem_D = 0;
  for (const auto i : make_range(_dim))
  {
    mooseAssert(elem_a(i).value() != 0, "We should not be dividing by zero");
    elem_D(i) = elem_volume / elem_a(i);
  }

  VectorValue<ADReal> face_D;

  if (neighbor && this->hasBlocks(neighbor->subdomain_id()))
  {
    const auto & neighbor_mu = _mu_neighbor[_qp];

    const VectorValue<ADReal> & neighbor_a = rcCoeff(*neighbor, neighbor_mu);

    coordTransformFactor(_subproblem, neighbor->subdomain_id(), *neighbor_centroid, coord);
    neighbor_volume *= coord;

    VectorValue<ADReal> neighbor_D = 0;
    for (const auto i : make_range(_dim))
    {
      mooseAssert(neighbor_a(i).value() != 0, "We should not be dividing by zero");
      neighbor_D(i) = neighbor_volume / neighbor_a(i);
    }
    Moose::FV::interpolate(
        Moose::FV::InterpMethod::Average, face_D, elem_D, neighbor_D, *_face_info, true);
  }
  else
    face_D = elem_D;

  // perform the pressure correction
  for (const auto i : make_range(_dim))
    v(i) -= face_D(i) * (grad_p(i) - unc_grad_p(i));
}
#else

VectorValue<ADReal>
INSFVMomentumAdvection::coeffCalculator(const Elem &, const ADReal &) const
{
  mooseError("INSFVMomentumAdvection only works with global AD indexing");
}

void
INSFVMomentumAdvection::interpolate(Moose::FV::InterpMethod,
                                    ADRealVectorValue &,
                                    const ADRealVectorValue &,
                                    const ADRealVectorValue &)
{
  mooseError("INSFVMomentumAdvection only works with global AD indexing");
}
#endif

ADReal
INSFVMomentumAdvection::computeQpResidual()
{
  ADRealVectorValue v;
  ADReal adv_quant_interface;

  this->interpolate(_velocity_interp_method, v, _vel_elem[_qp], _vel_neighbor[_qp]);
  Moose::FV::interpolate(_advected_interp_method,
                         adv_quant_interface,
                         _adv_quant_elem[_qp],
                         _adv_quant_neighbor[_qp],
                         v,
                         *_face_info,
                         true);
  return _normal * v * adv_quant_interface;
}

void
INSFVMomentumAdvection::clearRCCoeffs()
{
  auto it = _rc_a_coeffs.find(&_app);
  mooseAssert(it != _rc_a_coeffs.end(),
              "No RC coeffs structure exists for the given MooseApp pointer");
  mooseAssert(_tid < it->second.size(),
              "The RC coeffs structure size "
                  << it->second.size() << " is greater than or equal to the provided thread ID "
                  << _tid);
  it->second[_tid].clear();
}
