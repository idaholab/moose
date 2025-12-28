//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MassContinuityAssemblyHelper.h"
#include "MooseTypes.h"
#include "MooseObject.h"
#include "MaterialPropertyInterface.h"
#include "MooseMesh.h"

using namespace libMesh;

InputParameters
MassContinuityAssemblyHelper::validParams()
{
  auto params = IPHDGAssemblyHelper::validParams();
  params.addRequiredParam<std::vector<NonlinearVariableName>>(
      "interior_velocity_vars", "The velocity variables on the element interiors");
  params.addRequiredParam<std::vector<MooseFunctorName>>(
      "face_velocity_functors", "The velocity variables on element faces/facets");
  return params;
}

MassContinuityAssemblyHelper::MassContinuityAssemblyHelper(
    const MooseObject * const moose_obj,
    MooseVariableDependencyInterface * const mvdi,
    const TransientInterface * const ti,
    const MooseMesh & mesh,
    SystemBase & sys,
    const Assembly & assembly,
    const THREAD_ID tid,
    const std::set<SubdomainID> & block_ids,
    const std::set<BoundaryID> & boundary_ids)
  : IPHDGAssemblyHelper(moose_obj, mvdi, ti, sys, assembly, tid, block_ids, boundary_ids),
    ADFunctorInterface(moose_obj),
    _coord_sys(assembly.coordSystem()),
    _rz_radial_coord(mesh.getAxisymmetricRadialCoord())

{
  for (const auto & interior_vel_var_name :
       moose_obj->getParam<std::vector<NonlinearVariableName>>("interior_velocity_vars"))
  {
    const auto & var = sys.getFieldVariable<Real>(tid, interior_vel_var_name);
    _interior_vels.push_back(&var.adSln());
    _interior_vel_grads.push_back(&var.adGradSln());
  }
  for (const auto & face_vel_functor_name :
       moose_obj->getParam<std::vector<MooseFunctorName>>("face_velocity_functors"))
    _face_vels.push_back(&getFunctorByName<ADReal>(face_vel_functor_name));

  if (mesh.dimension() != _interior_vels.size())
    moose_obj->paramError(
        "interior_velocity_vars",
        "The number of interior velocity variables must be equal to the mesh dimension");
  if (mesh.dimension() != _face_vels.size())
    moose_obj->paramError(
        "face_velocity_functors",
        "The number of face velocity functors must be equal to the mesh dimension");
}

void
MassContinuityAssemblyHelper::scalarVolume()
{
  for (const auto qp : make_range(_ip_qrule->n_points()))
  {
    ADReal divergence = 0;
    for (const auto d : index_range(_interior_vel_grads))
    {
      divergence += (*_interior_vel_grads[d])[qp](d);
      if (_coord_sys == Moose::COORD_RZ && (_rz_radial_coord == d))
        divergence += (*_interior_vels[d])[qp] / _ip_q_point[qp](_rz_radial_coord);
    }
    const auto qp_term = _ip_JxW[qp] * divergence;
    for (const auto i : index_range(_scalar_phi))
      _scalar_re(i) -= qp_term * _scalar_phi[i][qp];
  }
}

void
MassContinuityAssemblyHelper::scalarFace()
{
}

void
MassContinuityAssemblyHelper::lmFace()
{
  for (const auto qp : make_range(_ip_qrule_face->n_points()))
  {
    ADRealVectorValue interior_vel, face_vel;
    for (const auto d : index_range(_interior_vels))
    {
      interior_vel(d) = (*_interior_vels[d])[qp];
      face_vel(d) = (*_face_vels[d])(
          Moose::ElemSideQpArg{
              _ip_current_elem, _ip_current_side, qp, _ip_qrule_face, _ip_q_point_face[qp]},
          _ti.determineState());
    }
    const auto qp_term = (interior_vel - face_vel) * _ip_normals[qp] * _ip_JxW_face[qp];
    for (const auto i : index_range(_lm_re))
      _lm_re(i) += _lm_phi_face[i][qp] * qp_term;
  }
}
