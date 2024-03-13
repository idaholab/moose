//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NavierStokesHDGAssemblyHelper.h"
#include "MooseVariableFE.h"
#include "MooseVariableScalar.h"
#include "Function.h"
#include "SystemBase.h"
#include "MooseMesh.h"
#include "MooseObject.h"
#include "MaterialPropertyInterface.h"
#include "NS.h"

#include "libmesh/point.h"
#include "libmesh/quadrature.h"
#include "libmesh/elem.h"

using namespace libMesh;

InputParameters
NavierStokesHDGAssemblyHelper::validParams()
{
  auto params = DiffusionHDGAssemblyHelper::validParams();
  params.addRequiredParam<NonlinearVariableName>(NS::pressure, "The pressure variable.");
  params.setDocString("u", "The x-component of velocity");
  params.addRequiredParam<AuxVariableName>("v", "The y-component of velocity");
  params.addParam<AuxVariableName>("w", "The z-component of velocity");
  params.setDocString("grad_u", "The gradient of the x-component of velocity");
  params.addRequiredParam<AuxVariableName>("grad_v", "The gradient of the y-component of velocity");
  params.addParam<AuxVariableName>("grad_w", "The gradient of the z-component of velocity");
  params.setDocString("face_u", "The x-component of the face velocity");
  params.addRequiredParam<NonlinearVariableName>("face_v", "The y-component of the face velocity");
  params.addParam<NonlinearVariableName>("face_w", "The z-component of the face velocity");
  params.addParam<NonlinearVariableName>(
      "enclosure_lm",
      "For enclosed problems like the lid driven cavity this variable can be provided to remove "
      "the pressure nullspace");
  params.renameParam("diffusivity", "nu", "The kinematic viscosity");
  return params;
}

NavierStokesHDGAssemblyHelper::NavierStokesHDGAssemblyHelper(const MooseObject * const moose_obj,
                                                             MaterialPropertyInterface * const mpi,
                                                             SystemBase & nl_sys,
                                                             SystemBase & aux_sys,
                                                             const MooseMesh & mesh,
                                                             const THREAD_ID tid)
  : DiffusionHDGAssemblyHelper(moose_obj, mpi, nl_sys, aux_sys, tid),
    // vars
    _v_var(aux_sys.getFieldVariable<Real>(tid, moose_obj->getParam<AuxVariableName>("v"))),
    _w_var(mesh.dimension() > 2
               ? &aux_sys.getFieldVariable<Real>(tid, moose_obj->getParam<AuxVariableName>("w"))
               : nullptr),
    _grad_v_var(aux_sys.getFieldVariable<RealVectorValue>(
        tid, moose_obj->getParam<AuxVariableName>("grad_v"))),
    _grad_w_var(mesh.dimension() > 2 ? &aux_sys.getFieldVariable<RealVectorValue>(
                                           tid, moose_obj->getParam<AuxVariableName>("grad_w"))
                                     : nullptr),
    _v_face_var(
        nl_sys.getFieldVariable<Real>(tid, moose_obj->getParam<NonlinearVariableName>("face_v"))),
    _w_face_var(mesh.dimension() > 2
                    ? &nl_sys.getFieldVariable<Real>(
                          tid, moose_obj->getParam<NonlinearVariableName>("face_w"))
                    : nullptr),
    _pressure_var(nl_sys.getFieldVariable<Real>(
        tid, moose_obj->getParam<NonlinearVariableName>(NS::pressure))),
    _enclosure_lm_var(moose_obj->isParamValid("enclosure_lm")
                          ? &nl_sys.getScalarVariable(
                                tid, moose_obj->getParam<NonlinearVariableName>("enclosure_lm"))
                          : nullptr),
    // dof indices
    _qv_dof_indices(_grad_v_var.dofIndices()),
    _v_dof_indices(_v_var.dofIndices()),
    _lm_v_dof_indices(_v_face_var.dofIndices()),
    _qw_dof_indices(_grad_w_var ? &_grad_w_var->dofIndices() : nullptr),
    _w_dof_indices(_w_var ? &_w_var->dofIndices() : nullptr),
    _lm_w_dof_indices(_w_face_var ? &_w_face_var->dofIndices() : nullptr),
    _p_dof_indices(_pressure_var.dofIndices()),
    _global_lm_dof_indices(_enclosure_lm_var ? &_enclosure_lm_var->dofIndices() : nullptr),
    // solutions
    _qv_sol(_grad_v_var.sln()),
    _v_sol(_v_var.sln()),
    _lm_v_sol(_v_face_var.sln()),
    _qw_sol(_grad_w_var ? &_grad_w_var->sln() : nullptr),
    _w_sol(_w_var ? &_w_var->sln() : nullptr),
    _lm_w_sol(_w_face_var ? &_w_face_var->sln() : nullptr),
    _p_sol(_pressure_var.sln()),
    _global_lm_dof_value(_enclosure_lm_var ? &_enclosure_lm_var->sln() : nullptr),
    // initialize local number of dofs
    _p_n_dofs(0),
    _global_lm_n_dofs(_enclosure_lm_var ? 1 : 0)
{
  if (mesh.dimension() > 2)
    mooseError("3D not yet implemented");
}
