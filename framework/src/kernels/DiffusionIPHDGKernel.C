//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "DiffusionIPHDGAssemblyHelper.h"
#include "DiffusionIPHDGKernel.h"
#include "MooseVariableFE.h"
#include "MooseVariableScalar.h"
#include "Function.h"
#include "SystemBase.h"
#include "MooseMesh.h"
#include "MooseObject.h"
#include "MaterialPropertyInterface.h"
#include "NonlinearThread.h"

using namespace libMesh;

registerMooseObject("MooseApp", DiffusionIPHDGKernel);

InputParameters
DiffusionIPHDGKernel::validParams()
{
  auto params = Kernel::validParams();
  params += DiffusionIPHDGAssemblyHelper::validParams();
  params.renameParam("variable", "u", "The diffusing specie concentration");
  params.addParam<MooseFunctorName>("source", 0, "Source for the diffusing species");
  return params;
}

DiffusionIPHDGKernel::DiffusionIPHDGKernel(const InputParameters & params)
  : Kernel(params),
    DiffusionIPHDGAssemblyHelper(this, this, this, this, _sys, _assembly, _tid),
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
DiffusionIPHDGKernel::compute()
{
  _scalar_re.resize(_u_dof_indices.size());
  _lm_re.resize(_lm_u_dof_indices.size());

  // u
  scalarVolume(_grad_u_sol, _source, _JxW, *_qrule, _current_elem, _q_point, _scalar_re);

  for (const auto side : _current_elem->side_index_range())
    if (_neigh = _current_elem->neighbor_ptr(side);
        _neigh && this->hasBlocks(_neigh->subdomain_id()))
    {
      NonlinearThread::prepareFace(
          _fe_problem, _tid, _current_elem, side, Moose::INVALID_BOUNDARY_ID);
      mooseAssert(_current_side == side, "The sides should be the same");
      // u, lm_u
      scalarFace(_grad_u_sol, _u_sol, _lm_u_sol, _JxW_face, *_qrule_face, _normals, _scalar_re);
      lmFace(_grad_u_sol, _u_sol, _lm_u_sol, _JxW_face, *_qrule_face, _normals, _lm_re);
    }
}

void
DiffusionIPHDGKernel::computeResidual()
{
  compute();
  addResiduals(_assembly, _scalar_re, _u_dof_indices, _u_var.scalingFactor());
  addResiduals(_assembly, _lm_re, _lm_u_dof_indices, _u_face_var.scalingFactor());
}

void
DiffusionIPHDGKernel::computeJacobian()
{
  compute();
  addJacobian(_assembly, _scalar_re, _u_dof_indices, _u_var.scalingFactor());
  addJacobian(_assembly, _lm_re, _lm_u_dof_indices, _u_face_var.scalingFactor());
}

void
DiffusionIPHDGKernel::computeResidualAndJacobian()
{
  compute();
  addResidualsAndJacobian(_assembly, _scalar_re, _u_dof_indices, _u_var.scalingFactor());
  addResidualsAndJacobian(_assembly, _lm_re, _lm_u_dof_indices, _u_face_var.scalingFactor());
}

void
DiffusionIPHDGKernel::jacobianSetup()
{
  _my_elem = nullptr;
}

void
DiffusionIPHDGKernel::computeOffDiagJacobian(const unsigned int)
{
  if (_my_elem != _current_elem)
  {
    computeJacobian();
    _my_elem = _current_elem;
  }
}

std::set<std::string>
DiffusionIPHDGKernel::additionalVariablesCovered()
{
  return {_u_var.name(), _u_face_var.name()};
}
