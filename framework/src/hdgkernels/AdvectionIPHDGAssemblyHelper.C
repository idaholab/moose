//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ActionWarehouse.h"
#include "AdvectionIPHDGAssemblyHelper.h"
#include "MooseTypes.h"
#include "MooseVariableDependencyInterface.h"
#include "MooseVariableFE.h"
#include "MooseVariableScalar.h"
#include "Function.h"
#include "SystemBase.h"
#include "MooseMesh.h"
#include "MooseObject.h"
#include "MaterialPropertyInterface.h"
#include "NonlinearThread.h"

using namespace libMesh;

InputParameters
AdvectionIPHDGAssemblyHelper::validParams()
{
  auto params = IPHDGAssemblyHelper::validParams();
  params.addRequiredParam<MaterialPropertyName>("velocity", "Velocity vector");
  params.addParam<MaterialPropertyName>(
      "advected_quantity",
      "An optional material property to be advected. If not "
      "supplied, then the primal/trace variable will be advected with upwinding.");
  return params;
}

AdvectionIPHDGAssemblyHelper::AdvectionIPHDGAssemblyHelper(
    const MooseObject * const moose_obj,
    MooseVariableDependencyInterface * const mvdi,
    const TransientInterface * const ti,
    SystemBase & sys,
    const Assembly & assembly,
    const THREAD_ID tid,
    const std::set<SubdomainID> & block_ids,
    const std::set<BoundaryID> & boundary_ids)
  : IPHDGAssemblyHelper(moose_obj, mvdi, ti, sys, assembly, tid, block_ids, boundary_ids),
    _velocity(getADMaterialProperty<RealVectorValue>("velocity")),
    _face_velocity(getFaceADMaterialProperty<RealVectorValue>("velocity")),
    _adv_quant(moose_obj->isParamValid("advected_quantity")
                   ? &getADMaterialProperty<Real>("advected_quantity")
                   : nullptr),
    _adv_quant_face(moose_obj->isParamValid("advected_quantity")
                        ? &getFaceADMaterialProperty<Real>("advected_quantity")
                        : nullptr)
{
}

void
AdvectionIPHDGAssemblyHelper::scalarVolume()
{
  for (const auto qp : make_range(_ip_qrule->n_points()))
  {
    const auto & adv_quant = _adv_quant ? (*_adv_quant)[qp] : _u_sol[qp];
    for (const auto i : index_range(_scalar_re))
      _scalar_re(i) -= _ip_JxW[qp] * _grad_scalar_phi[i][qp] * _velocity[qp] * adv_quant;
  }
}

void
AdvectionIPHDGAssemblyHelper::scalarFace()
{
  for (const auto qp : make_range(_ip_qrule_face->n_points()))
  {
    const auto vdotn = _face_velocity[qp] * _ip_normals[qp];
    const auto & adv_quant =
        _adv_quant_face ? (*_adv_quant_face)[qp]
                        : (MetaPhysicL::raw_value(vdotn) >= 0 ? _u_sol[qp] : _lm_u_sol[qp]);
    for (const auto i : index_range(_scalar_re))
      _scalar_re(i) += _ip_JxW_face[qp] * _scalar_phi_face[i][qp] * vdotn * adv_quant;
  }
}

void
AdvectionIPHDGAssemblyHelper::lmFace()
{
  for (const auto qp : make_range(_ip_qrule_face->n_points()))
  {
    const auto vdotn = _face_velocity[qp] * _ip_normals[qp];
    const auto & adv_quant =
        _adv_quant_face ? (*_adv_quant_face)[qp]
                        : (MetaPhysicL::raw_value(vdotn) >= 0 ? _u_sol[qp] : _lm_u_sol[qp]);
    for (const auto i : index_range(_lm_re))
      _lm_re(i) -= _ip_JxW_face[qp] * _lm_phi_face[i][qp] * vdotn * adv_quant;
  }
}

void
AdvectionIPHDGAssemblyHelper::scalarDirichlet(const Moose::Functor<Real> & dirichlet_functor)
{
  for (const auto qp : make_range(_ip_qrule_face->n_points()))
  {
    const auto vdotn = _face_velocity[qp] * _ip_normals[qp];
    mooseAssert(MetaPhysicL::raw_value(vdotn) <= 0, "The velocity must create inflow conditions");
    const auto dirichlet_value = dirichlet_functor(
        Moose::ElemSideQpArg{
            _ip_current_elem, _ip_current_side, qp, _ip_qrule_face, _ip_q_point_face[qp]},
        _ti.determineState());
    for (const auto i : index_range(_scalar_re))
      _scalar_re(i) += _ip_JxW_face[qp] * _scalar_phi_face[i][qp] * vdotn * dirichlet_value;
  }
}

void
AdvectionIPHDGAssemblyHelper::lmOutflow()
{
  for (const auto qp : make_range(_ip_qrule_face->n_points()))
  {
#ifndef NDEBUG
    const auto vdotn = _face_velocity[qp] * _ip_normals[qp];
    mooseAssert(MetaPhysicL::raw_value(vdotn) >= 0, "The velocity must create outflow conditions");
#endif
    for (const auto i : index_range(_lm_re))
      _lm_re(i) += _ip_JxW_face[qp] * (_lm_u_sol[qp] - _u_sol[qp]) * _lm_phi_face[i][qp];
  }
}
