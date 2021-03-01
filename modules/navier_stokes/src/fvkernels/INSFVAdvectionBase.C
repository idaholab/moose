//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "INSFVAdvectionBase.h"

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

std::unordered_map<const MooseApp *,
                   std::vector<std::unordered_map<const Elem *, VectorValue<ADReal>>>>
    INSFVAdvectionBase::_rc_a_coeffs;

InputParameters
INSFVAdvectionBase::validParams()
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

  // We need 2 ghost layers for the Rhie-Chow interpolation
  params.set<unsigned short>("ghost_layers") = 2;

  params.addClassDescription("Base class for advecting momentum, e.g. rho*u");

  return params;
}

INSFVAdvectionBase::INSFVAdvectionBase(const InputParameters & params)
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
INSFVAdvectionBase::initialSetup()
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
INSFVAdvectionBase::setupFlowBoundaries(const BoundaryID bnd_id)
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
INSFVAdvectionBase::setupBoundaries(const BoundaryID bnd_id,
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
INSFVAdvectionBase::skipForBoundary(const FaceInfo & fi) const
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
INSFVAdvectionBase::rcCoeff(const Elem & elem, const ADReal & mu) const
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
void
INSFVAdvectionBase::interpolate(Moose::FV::InterpMethod m,
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

  if (onBoundary(*_face_info))
  {
    // In my mind there should only be about one bc_id per FaceInfo
    mooseAssert(_face_info->boundaryIDs().size() == 1,
                "I think some of my logic might depend on my implicit assumption that we have "
                "one boundary ID at most per face");
    mooseAssert(_flow_boundaries.find(*_face_info->boundaryIDs().begin()) != _flow_boundaries.end(),
                "INSFV*Advection flux kernel objects should only execute on flow boundaries.");

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

void
INSFVAdvectionBase::interpolate(Moose::FV::InterpMethod,
                                ADRealVectorValue &,
                                const ADRealVectorValue &,
                                const ADRealVectorValue &)
{
}
#endif

ADReal
INSFVAdvectionBase::computeQpResidual()
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
INSFVAdvectionBase::clearRCCoeffs()
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
