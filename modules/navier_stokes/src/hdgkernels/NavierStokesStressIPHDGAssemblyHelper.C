//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NavierStokesStressIPHDGAssemblyHelper.h"
#include "MooseTypes.h"
#include "MooseVariableDependencyInterface.h"
#include "MooseVariableScalar.h"
#include "SystemBase.h"
#include "MooseMesh.h"
#include "MooseObject.h"
#include "MaterialPropertyInterface.h"
#include "Assembly.h"
#include "MooseMesh.h"

using namespace libMesh;

InputParameters
NavierStokesStressIPHDGAssemblyHelper::validParams()
{
  auto params = DiffusionIPHDGAssemblyHelper::validParams();
  params.addRequiredParam<NonlinearVariableName>(
      "pressure_variable", "The pressure variable that lives on element interiors.");
  params.addRequiredParam<NonlinearVariableName>(
      "pressure_face_variable", "The pressure variable that lives on element faces.");
  params.addRequiredParam<unsigned int>("component", "number of component (0 = x, 1 = y, 2 = z)");
  return params;
}

NavierStokesStressIPHDGAssemblyHelper::NavierStokesStressIPHDGAssemblyHelper(
    const MooseObject * const moose_obj,
    MooseVariableDependencyInterface * const mvdi,
    const TransientInterface * const ti,
    const MooseMesh & mesh,
    SystemBase & sys,
    const Assembly & assembly,
    const THREAD_ID tid,
    const std::set<SubdomainID> & block_ids,
    const std::set<BoundaryID> & boundary_ids)
  : DiffusionIPHDGAssemblyHelper(moose_obj, mvdi, ti, sys, assembly, tid, block_ids, boundary_ids),
    _pressure_var(sys.getFieldVariable<Real>(
        tid, moose_obj->getParam<NonlinearVariableName>("pressure_variable"))),
    _pressure_face_var(sys.getFieldVariable<Real>(
        tid, moose_obj->getParam<NonlinearVariableName>("pressure_face_variable"))),
    _pressure_sol(_pressure_var.adSln()),
    _pressure_face_sol(_pressure_face_var.adSln()),
    _coord_sys(assembly.coordSystem()),
    _rz_radial_coord(mesh.getAxisymmetricRadialCoord()),
    _component(moose_obj->getParam<unsigned int>("component"))
{
}

void
NavierStokesStressIPHDGAssemblyHelper::scalarVolume()
{
  DiffusionIPHDGAssemblyHelper::scalarVolume();

  for (const auto qp : make_range(_ip_qrule->n_points()))
    for (const auto i : index_range(_scalar_re))
    {
      _scalar_re(i) -= _ip_JxW[qp] * (_grad_scalar_phi[i][qp](_component) * _pressure_sol[qp]);
      if (_coord_sys == Moose::COORD_RZ && (_rz_radial_coord == _component))
        _scalar_re(i) -= _pressure_sol[qp] / _ip_q_point[qp](_rz_radial_coord) * _scalar_phi[i][qp];
    }
}

void
NavierStokesStressIPHDGAssemblyHelper::scalarFace()
{
  DiffusionIPHDGAssemblyHelper::scalarFace();

  for (const auto i : index_range(_scalar_re))
    for (const auto qp : make_range(_ip_qrule_face->n_points()))
      _scalar_re(i) += _ip_JxW_face[qp] * _pressure_face_sol[qp] * _ip_normals[qp](_component) *
                       _scalar_phi_face[i][qp];
}

void
NavierStokesStressIPHDGAssemblyHelper::scalarDirichlet(const Moose::Functor<Real> & dirichlet_value)
{
  DiffusionIPHDGAssemblyHelper::scalarDirichlet(dirichlet_value);

  for (const auto i : index_range(_scalar_re))
    for (const auto qp : make_range(_ip_qrule_face->n_points()))
      _scalar_re(i) += _ip_JxW_face[qp] * _pressure_face_sol[qp] * _ip_normals[qp](_component) *
                       _scalar_phi_face[i][qp];
}
