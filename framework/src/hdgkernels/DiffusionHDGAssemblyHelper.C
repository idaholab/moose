//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "DiffusionHDGAssemblyHelper.h"
#include "MooseVariableFE.h"
#include "MooseVariableScalar.h"
#include "Function.h"
#include "SystemBase.h"
#include "MooseMesh.h"
#include "MooseObject.h"
#include "MaterialPropertyInterface.h"

using namespace libMesh;

InputParameters
DiffusionHDGAssemblyHelper::validParams()
{
  auto params = emptyInputParameters();
  params.addRequiredParam<AuxVariableName>("u", "The diffusing specie concentration");
  params.addRequiredParam<AuxVariableName>("grad_u",
                                           "The gradient of the diffusing specie concentration");
  params.addRequiredParam<NonlinearVariableName>(
      "face_u", "The concentration of the diffusing specie on faces");
  params.addRequiredParam<MaterialPropertyName>("diffusivity", "The diffusivity");
  return params;
}

DiffusionHDGAssemblyHelper::DiffusionHDGAssemblyHelper(const MooseObject * const moose_obj,
                                                       MaterialPropertyInterface * const mpi,
                                                       SystemBase & nl_sys,
                                                       SystemBase & aux_sys,
                                                       const THREAD_ID tid)
  : // vars
    _u_var(aux_sys.getFieldVariable<Real>(tid, moose_obj->getParam<AuxVariableName>("u"))),
    _grad_u_var(aux_sys.getFieldVariable<RealVectorValue>(
        tid, moose_obj->getParam<AuxVariableName>("grad_u"))),
    _u_face_var(
        nl_sys.getFieldVariable<Real>(tid, moose_obj->getParam<NonlinearVariableName>("face_u"))),
    // dof indices
    _qu_dof_indices(_grad_u_var.dofIndices()),
    _u_dof_indices(_u_var.dofIndices()),
    _lm_u_dof_indices(_u_face_var.dofIndices()),
    // solutions
    _qu_sol(_grad_u_var.sln()),
    _u_sol(_u_var.sln()),
    _lm_u_sol(_u_face_var.sln()),
    // shape functions
    _vector_phi(_grad_u_var.phi()),
    _scalar_phi(_u_var.phi()),
    _grad_scalar_phi(_u_var.gradPhi()),
    _div_vector_phi(_grad_u_var.divPhi()),
    _vector_phi_face(_grad_u_var.phiFace()),
    _scalar_phi_face(_u_var.phiFace()),
    _lm_phi_face(_u_face_var.phiFace()),
    // material properties
    _diff(mpi->getMaterialProperty<Real>("diffusivity")),
    // initialize local number of dofs
    _vector_n_dofs(0),
    _scalar_n_dofs(0),
    _lm_n_dofs(0)
{
}
