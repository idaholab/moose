//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "DiffusionIPHDGDirichletBC.h"
#include "MooseVariableFE.h"

registerMooseObject("MooseApp", DiffusionIPHDGDirichletBC);

InputParameters
DiffusionIPHDGDirichletBC::validParams()
{
  auto params = IntegratedBC::validParams();
  params.addClassDescription("Weakly imposes Dirichlet boundary conditions for a "
                             "hybridized discretization of a diffusion equation");
  params.addParam<MooseFunctorName>("functor", 0, "The Dirichlet value for the diffusing specie");
  params += DiffusionIPHDGAssemblyHelper::validParams();
  params.renameParam("variable", "u", "The diffusing specie concentration");
  return params;
}

DiffusionIPHDGDirichletBC::DiffusionIPHDGDirichletBC(const InputParameters & parameters)
  : IntegratedBC(parameters),
    DiffusionIPHDGAssemblyHelper(this, this, this, this, _sys, _assembly, _tid),
    _dirichlet_val(getFunctor<Real>("functor")),
    _my_side(libMesh::invalid_uint)
{
}

void
DiffusionIPHDGDirichletBC::compute()
{
  _scalar_re.resize(_u_dof_indices.size());
  _lm_re.resize(_lm_u_dof_indices.size());

  // u, lm_u
  scalarDirichlet(_grad_u_sol,
                  _lm_u_sol,
                  _dirichlet_val,
                  _JxW,
                  *_qrule,
                  _normals,
                  _current_elem,
                  _current_side,
                  _q_point,
                  _scalar_re);
  lmDirichlet(_grad_u_sol,
              _lm_u_sol,
              _dirichlet_val,
              _JxW,
              *_qrule,
              _normals,
              _current_elem,
              _current_side,
              _q_point,
              _lm_re);
}

void
DiffusionIPHDGDirichletBC::computeResidual()
{
  compute();
  addResiduals(_assembly, _scalar_re, _u_dof_indices, _u_var.scalingFactor());
  addResiduals(_assembly, _lm_re, _lm_u_dof_indices, _u_face_var.scalingFactor());
}

void
DiffusionIPHDGDirichletBC::computeJacobian()
{
  compute();
  addJacobian(_assembly, _scalar_re, _u_dof_indices, _u_var.scalingFactor());
  addJacobian(_assembly, _lm_re, _lm_u_dof_indices, _u_face_var.scalingFactor());
}

void
DiffusionIPHDGDirichletBC::computeResidualAndJacobian()
{
  compute();
  addResidualsAndJacobian(_assembly, _scalar_re, _u_dof_indices, _u_var.scalingFactor());
  addResidualsAndJacobian(_assembly, _lm_re, _lm_u_dof_indices, _u_face_var.scalingFactor());
}

void
DiffusionIPHDGDirichletBC::jacobianSetup()
{
  _my_elem = nullptr;
  _my_side = libMesh::invalid_uint;
}

void
DiffusionIPHDGDirichletBC::computeOffDiagJacobian(const unsigned int)
{
  if ((_my_elem != _current_elem) || (_my_side != _current_side))
  {
    computeJacobian();
    _my_elem = _current_elem;
    _my_side = _current_side;
  }
}
