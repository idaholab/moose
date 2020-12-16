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

  params.addParam<std::vector<BoundaryName>>(
      "no_slip_wall_boundaries", std::vector<BoundaryName>(), "No slip wall boundaries.");
  params.addParam<std::vector<BoundaryName>>(
      "slip_wall_boundaries", std::vector<BoundaryName>(), "Slip wall boundaries.");
  params.addParam<std::vector<BoundaryName>>(
      "flow_boundaries", std::vector<BoundaryName>(), "Flow boundaries.");

  params.addClassDescription("Object for advecting momentum, e.g. rho*u");

  return params;
}

INSFVMomentumAdvection::INSFVMomentumAdvection(const InputParameters & params)
  : FVMatAdvection(params),
    _mu_elem(getADMaterialProperty<Real>("mu")),
    _mu_neighbor(getNeighborADMaterialProperty<Real>("mu")),
    _p_var(dynamic_cast<const MooseVariableFV<Real> *>(
        &_subproblem.getVariable(_tid, params.get<std::vector<VariableName>>("pressure").front()))),
    _u_var(dynamic_cast<const MooseVariableFV<Real> *>(
        &_subproblem.getVariable(_tid, params.get<std::vector<VariableName>>("u").front()))),
    _v_var(params.isParamValid("v")
               ? dynamic_cast<const MooseVariableFV<Real> *>(&_subproblem.getVariable(
                     _tid, params.get<std::vector<VariableName>>("v").front()))
               : nullptr),
    _w_var(params.isParamValid("w")
               ? dynamic_cast<const MooseVariableFV<Real> *>(&_subproblem.getVariable(
                     _tid, params.get<std::vector<VariableName>>("w").front()))
               : nullptr),
    _rho(params.get<Real>("rho")),
    _dim(_subproblem.mesh().dimension())
{
#ifndef MOOSE_GLOBAL_AD_INDEXING
  mooseError("INSFV is not supported by local AD indexing. In order to use INSFV, please run the "
             "configure script in the root MOOSE directory with the configure option "
             "'--with-ad-indexing-type=global'");
#endif

  if (!_p_var)
    mooseError("the pressure must be a finite volume variable.");

  if (!_u_var)
    mooseError("the u velocity must be a finite volume variable.");

  if (_dim >= 2 && !_v_var)
    mooseError(
        "In two or more dimensions, the v velocity must be supplied and it must be a finite volume "
        "variable.");

  if (_dim >= 3 && !params.isParamValid("w"))
    mooseError("In three-dimensions, the w velocity must be supplied and it must be a finite "
               "volume variable.");

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

  {
    // Setup no slip wall boundary IDs
    const auto & vec = getParam<std::vector<BoundaryName>>("no_slip_wall_boundaries");
    for (const auto & name : vec)
      _no_slip_wall_boundaries.insert(_mesh.getBoundaryID(name));
  }
  {
    // Setup slip wall boundary IDs
    const auto & vec = getParam<std::vector<BoundaryName>>("slip_wall_boundaries");
    for (const auto & name : vec)
      _slip_wall_boundaries.insert(_mesh.getBoundaryID(name));
  }
  {
    // Setup flow boundary IDs
    const auto & vec = getParam<std::vector<BoundaryName>>("flow_boundaries");
    for (const auto & name : vec)
      _flow_boundaries.insert(_mesh.getBoundaryID(name));
  }

  std::set<BoundaryID> sum = _no_slip_wall_boundaries;
  for (const auto id : _slip_wall_boundaries)
  {
    const auto pr = sum.emplace(id);
    if (!pr.second)
      mooseError(
          "Duplicate coverage of ", id, " by no_slip_wall_boundaries and slip_wall_boundaries.");
  }
  for (const auto id : _flow_boundaries)
  {
    const auto pr = sum.emplace(id);
    if (!pr.second)
      mooseError(
          "Duplicate coverage of ",
          id,
          " by flow_boundaries and the union of no_slip_wall_boundaries and slip_wall_boundaries.");
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

bool
INSFVMomentumAdvection::skipForBoundary(const FaceInfo & fi) const
{
  if (!fi.isBoundary())
    return false;

  // If we have flux bcs then we do skip
  const auto & flux_pr = _var.getFluxBCs(fi);
  if (flux_pr.first)
    return true;

  // If we have dirichlet bcs then we don't skip.
  if (_var.getDirichletBC(fi).first)
    return false;

  // Finally if we have a flow boundary, we don't skip. Mass and momentum are transported via
  // advection across boundaries
  for (const auto bc_id : fi.boundaryIDs())
    if (_flow_boundaries.find(bc_id) != _flow_boundaries.end())
      return false;

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

  auto action_functor = [&coeff, &elem_velocity, &mu, this](const Elem & /*functor_elem*/,
                                                            const Elem * const neighbor,
                                                            const FaceInfo * const fi,
                                                            const Point & surface_vector,
                                                            Real /*coord*/,
                                                            const bool /*elem_has_info*/) {
    mooseAssert(fi, "We need a non-null FaceInfo");

    const bool rc_elem_is_fi_elem = (neighbor == fi->neighborPtr());
    const Point normal = rc_elem_is_fi_elem ? fi->normal() : Point(-fi->normal());

    // Unless specified otherwise, "elem" here refers to the element we're computing the
    // Rhie-Chow coefficient for. "neighbor" is the element across the current FaceInfo (fi)
    // face from the Rhie-Chow element

    if (fi->isBoundary())
    {
      // In my mind there should only be about one bc_id per FaceInfo
      for (const auto bc_id : fi->boundaryIDs())
      {
        if (_no_slip_wall_boundaries.find(bc_id) != _no_slip_wall_boundaries.end())
        {
          // Need to account for viscous shear stress from wall
          for (const auto i : make_range(_dim))
            coeff(i) += mu * surface_vector.norm() /
                        std::abs((fi->faceCentroid() - fi->elemCentroid()) * normal) *
                        (1 - normal(i) * normal(i));

          // No flow normal to wall, so no contribution to coefficient from the advection term
          break;
        }

        if (_flow_boundaries.find(bc_id) != _flow_boundaries.end())
        {
          ADRealVectorValue face_velocity(_u_var->getBoundaryFaceValue(*fi));
          if (_v_var)
            face_velocity(1) = _v_var->getBoundaryFaceValue(*fi);
          if (_w_var)
            face_velocity(2) = _w_var->getBoundaryFaceValue(*fi);

          const auto advection_coeffs = Moose::FV::interpCoeffs(
              _advected_interp_method, *fi, rc_elem_is_fi_elem, face_velocity);
          ADReal temp_coeff = _rho * face_velocity * surface_vector * advection_coeffs.first;

          // For flow boundaries, the coefficient addition is the same for every velocity component
          for (const auto i : make_range(_dim))
            coeff(i) += temp_coeff;

          // No wall exerting shear on the fluid, so no contribution to the coefficient from the
          // viscous term
          break;
        }

        if (_slip_wall_boundaries.find(bc_id) == _slip_wall_boundaries.end())
          mooseError(
              "If INSFVMomentumAdvection is to be executed along a boundary, then that boundary "
              "must be included in one of the following parameters: 'no_slip_wall_boundaries', "
              "'slip_wall_boundaries', or 'flow_boundaries'. However, the current boundary ID ",
              bc_id,
              " is not currently covered.");

        // In the case of a slip wall we neither have viscous shear stress from the wall nor normal
        // outflow, so our contribution to the coefficient is zero
      }
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
                             rc_elem_is_fi_elem);

      // we are only interested in the interpolation coefficient for the Rhie-Chow element,
      // so we just use the 'first' member of the returned pair
      const auto advection_coeffs =
          Moose::FV::interpCoeffs(_advected_interp_method, *fi, rc_elem_is_fi_elem, interp_v);
      ADReal temp_coeff = _rho * interp_v * surface_vector * advection_coeffs.first;

      // Now add the viscous flux. Note that this includes only the orthogonal component! See
      // Moukalled equations 8.80, 8.78, and the orthogonal correction approach equation for
      // E_f, equation 8.69
      temp_coeff +=
          mu * surface_vector.norm() / (fi->neighborCentroid() - fi->elemCentroid()).norm();

      // For internal faces the coefficient is the same for every velocity component.
      for (const auto i : make_range(_dim))
        coeff(i) += temp_coeff;
    }
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

  // Get uncorrected pressure gradient. This will use the element centroid gradient if we are
  // along a boundary face
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

  VectorValue<ADReal> elem_one_D = 0;
  for (const auto i : make_range(_dim))
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
    for (const auto i : make_range(_dim))
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
