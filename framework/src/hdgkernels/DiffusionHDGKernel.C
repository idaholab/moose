//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "DiffusionHDGAssemblyHelper.h"
#include "DiffusionHDGKernel.h"
#include "MooseVariableFE.h"
#include "MooseVariableScalar.h"
#include "Function.h"
#include "SystemBase.h"
#include "MooseMesh.h"
#include "MooseObject.h"
#include "MaterialPropertyInterface.h"
#include "NonlinearThread.h"

using namespace libMesh;

registerMooseObject("MooseApp", DiffusionHDGKernel);

InputParameters
DiffusionHDGKernel::validParams()
{
  auto params = HDGKernel::validParams();
  params += DiffusionHDGAssemblyHelper::validParams();
  params.renameParam("variable", "u", "The diffusing specie concentration");
  params.addParam<MooseFunctorName>("source", 0, "Source for the diffusing species");
  return params;
}

DiffusionHDGKernel::DiffusionHDGKernel(const InputParameters & params)
  : HDGKernel(params),
    DiffusionHDGAssemblyHelper(this, this, this, this, _fe_problem, _sys, _tid),
    _source(getFunctor<Real>("source")),
    _qrule_face(_assembly.qRuleFace()),
    _q_point_face(_assembly.qPointsFace()),
    _JxW_face(_assembly.JxWFace()),
    _coord_face(_assembly.coordTransformation()),
    _normals(_assembly.normals()),
    _current_side(_assembly.side())
{
}

void
DiffusionHDGKernel::initialSetup()
{
  // This check must occur after FEProblemBase::init()
  checkCoupling();
}

void
DiffusionHDGKernel::computeResidual()
{
  _vector_re.resize(_qu_dof_indices.size());
  _scalar_re.resize(_u_dof_indices.size());

  // qu and u
  vectorVolumeResidual(_qu_sol, _u_sol, _JxW, *_qrule, _vector_re);
  scalarVolumeResidual(_qu_sol, _source, _JxW, *_qrule, _current_elem, _q_point, _scalar_re);

  addResiduals(_assembly, _vector_re, _qu_dof_indices, _grad_u_var.scalingFactor());
  addResiduals(_assembly, _scalar_re, _u_dof_indices, _u_var.scalingFactor());
}

void
DiffusionHDGKernel::computeResidualOnSide()
{
  _vector_re.resize(_qu_dof_indices.size());
  _scalar_re.resize(_u_dof_indices.size());
  _lm_re.resize(_lm_u_dof_indices.size());

  // qu, u, lm_u
  vectorFaceResidual(_lm_u_sol, _JxW_face, *_qrule_face, _normals, _vector_re);
  scalarFaceResidual(_qu_sol, _u_sol, _lm_u_sol, _JxW_face, *_qrule_face, _normals, _scalar_re);
  lmFaceResidual(_qu_sol, _u_sol, _lm_u_sol, _JxW_face, *_qrule_face, _normals, _lm_re);

  addResiduals(_assembly, _vector_re, _qu_dof_indices, _grad_u_var.scalingFactor());
  addResiduals(_assembly, _scalar_re, _u_dof_indices, _u_var.scalingFactor());
  addResiduals(_assembly, _lm_re, _lm_u_dof_indices, _u_face_var.scalingFactor());
}

void
DiffusionHDGKernel::computeJacobian()
{
  _vector_vector_jac.resize(_qu_dof_indices.size(), _qu_dof_indices.size());
  _vector_scalar_jac.resize(_qu_dof_indices.size(), _u_dof_indices.size());
  _scalar_vector_jac.resize(_u_dof_indices.size(), _qu_dof_indices.size());

  // qu and u
  vectorVolumeJacobian(_JxW, *_qrule, _vector_vector_jac, _vector_scalar_jac);
  scalarVolumeJacobian(_JxW, *_qrule, _scalar_vector_jac);

  addJacobian(
      _assembly, _vector_vector_jac, _qu_dof_indices, _qu_dof_indices, _grad_u_var.scalingFactor());
  addJacobian(
      _assembly, _vector_scalar_jac, _qu_dof_indices, _u_dof_indices, _grad_u_var.scalingFactor());
  addJacobian(
      _assembly, _scalar_vector_jac, _u_dof_indices, _qu_dof_indices, _u_var.scalingFactor());
}

void
DiffusionHDGKernel::computeJacobianOnSide()
{
  _scalar_vector_jac.resize(_u_dof_indices.size(), _qu_dof_indices.size());
  _scalar_scalar_jac.resize(_u_dof_indices.size(), _u_dof_indices.size());
  _scalar_lm_jac.resize(_u_dof_indices.size(), _lm_u_dof_indices.size());
  _lm_scalar_jac.resize(_lm_u_dof_indices.size(), _u_dof_indices.size());
  _lm_lm_jac.resize(_lm_u_dof_indices.size(), _lm_u_dof_indices.size());
  _vector_lm_jac.resize(_qu_dof_indices.size(), _lm_u_dof_indices.size());
  _lm_vector_jac.resize(_lm_u_dof_indices.size(), _qu_dof_indices.size());

  // qu, u, lm_u
  vectorFaceJacobian(_JxW_face, *_qrule_face, _normals, _vector_lm_jac);
  scalarFaceJacobian(
      _JxW_face, *_qrule_face, _normals, _scalar_vector_jac, _scalar_scalar_jac, _scalar_lm_jac);
  lmFaceJacobian(_JxW_face, *_qrule_face, _normals, _lm_vector_jac, _lm_scalar_jac, _lm_lm_jac);

  addJacobian(
      _assembly, _scalar_vector_jac, _u_dof_indices, _qu_dof_indices, _u_var.scalingFactor());
  addJacobian(
      _assembly, _scalar_scalar_jac, _u_dof_indices, _u_dof_indices, _u_var.scalingFactor());
  addJacobian(_assembly, _scalar_lm_jac, _u_dof_indices, _lm_u_dof_indices, _u_var.scalingFactor());
  addJacobian(
      _assembly, _lm_scalar_jac, _lm_u_dof_indices, _u_dof_indices, _u_face_var.scalingFactor());
  addJacobian(
      _assembly, _lm_lm_jac, _lm_u_dof_indices, _lm_u_dof_indices, _u_face_var.scalingFactor());
  addJacobian(
      _assembly, _vector_lm_jac, _qu_dof_indices, _lm_u_dof_indices, _grad_u_var.scalingFactor());
  addJacobian(
      _assembly, _lm_vector_jac, _lm_u_dof_indices, _qu_dof_indices, _u_face_var.scalingFactor());
}

void
DiffusionHDGKernel::jacobianSetup()
{
  _my_elem = nullptr;
}

void
DiffusionHDGKernel::computeOffDiagJacobian(const unsigned int)
{
  if (_my_elem != _current_elem)
  {
    computeJacobian();
    _my_elem = _current_elem;
  }
}

std::set<std::string>
DiffusionHDGKernel::additionalVariablesCovered()
{
  return {_u_var.name(), _grad_u_var.name(), _u_face_var.name()};
}
