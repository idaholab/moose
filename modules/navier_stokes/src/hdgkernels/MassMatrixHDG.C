//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MassMatrixHDG.h"
#include "MassMatrix.h"

// MOOSE includes
#include "MooseVariableFE.h"

registerMooseObject("NavierStokesApp", MassMatrixHDG);

InputParameters
MassMatrixHDG::validParams()
{
  InputParameters params = HDGKernel::validParams();
  params.setDocString(
      "variable",
      "The facet variable for whom we will be computing the mass on the internal sides");
  params.addClassDescription(
      "Computes a finite element mass matrix on internal faces (element by "
      "element!) meant for use in preconditioning schemes which require one");
  MassMatrix::setMassMatrixParams(params);
  params.addParam<Real>("density", 1, "The density");
  return params;
}

MassMatrixHDG::MassMatrixHDG(const InputParameters & parameters)
  : HDGKernel(parameters), _face_phi(_var.phiFace()), _density(getParam<Real>("density"))
{
}

void
MassMatrixHDG::computeJacobianOnSide()
{
  mooseAssert(_face_phi.size() == _var.dofIndices().size(), "These should be the same size");
  // resize always zeroes for DenseMatrix
  _mass.resize(_face_phi.size(), _face_phi.size());
  for (const auto qp : make_range(_qrule_face->n_points()))
  {
    // The division by 2 here is necessary because each face will be visited twice, once from each
    // neighboring element
    const auto qp_quant = _JxW_face[qp] * _coord[qp] * _density / 2;
    for (const auto i : index_range(_face_phi))
    {
      const auto qp_i_quant = qp_quant * _face_phi[i][qp];
      for (const auto j : index_range(_face_phi))
        _mass(i, j) += qp_i_quant * _face_phi[j][qp];
    }
  }

  addJacobian(_assembly, _mass, _var.dofIndices(), _var.dofIndices(), _var.scalingFactor());
}
