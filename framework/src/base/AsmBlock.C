/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#include "AsmBlock.h"
#include "SystemBase.h"


AsmBlock::AsmBlock(SystemBase & sys, CouplingMatrix * & cm, THREAD_ID tid) :
    _sys(sys),
    _cm(cm),
    _dof_map(_sys.dofMap()),
    _tid(tid)
{
}

AsmBlock::~AsmBlock()
{
}

void
AsmBlock::init()
{
  unsigned int n_vars = _sys.nVariables();

  // I want the blocks to go by columns first to reduce copying of shape function in assembling "full" Jacobian
  _cm_entry.clear();
  for (unsigned int j = 0; j < n_vars; ++j)
    for (unsigned int i = 0; i < n_vars; ++i)
      if ((*_cm)(i, j) != 0)
        _cm_entry.push_back(std::pair<unsigned int, unsigned int>(i, j));

  _sub_Re.resize(n_vars);

  _sub_Ke.resize(n_vars);
  for (unsigned int i = 0; i < n_vars; ++i)
    _sub_Ke[i].resize(n_vars);
}

void
AsmBlock::prepare()
{
  for (std::vector<std::pair<unsigned int, unsigned int> >::iterator it = _cm_entry.begin(); it != _cm_entry.end(); ++it)
  {
    unsigned int vi = (*it).first;
    unsigned int vj = (*it).second;

    MooseVariable & ivar = _sys.getVariable(_tid, vi);
    MooseVariable & jvar = _sys.getVariable(_tid, vj);

    _sub_Ke[vi][vj].resize(ivar.dofIndices().size(), jvar.dofIndices().size());
    _sub_Ke[vi][vj].zero();
  }

  unsigned int n_vars = _sys.nVariables();
  for (unsigned int vi = 0; vi < n_vars; vi++)
  {
    MooseVariable & ivar = _sys.getVariable(_tid, vi);
    _sub_Re[vi].resize(ivar.dofIndices().size());
    _sub_Re[vi].zero();
  }
}

void
AsmBlock::prepareBlock(unsigned int ivar, unsigned jvar, const std::vector<unsigned int> & dof_indices)
{
  _sub_Ke[ivar][jvar].resize(dof_indices.size(), dof_indices.size());
  _sub_Ke[ivar][jvar].zero();

  _sub_Re[ivar].resize(dof_indices.size());
  _sub_Re[ivar].zero();
}

void
AsmBlock::copyShapes(unsigned int var)
{
  MooseVariable & v = _sys.getVariable(_tid, var);

  _phi = v.phi();
  _grad_phi = v.gradPhi();
  _second_phi = v.secondPhi();
}

void
AsmBlock::copyFaceShapes(unsigned int var)
{
  MooseVariable & v = _sys.getVariable(_tid, var);

  _phi_face = v.phiFace();
  _grad_phi_face = v.gradPhiFace();
  _second_phi_face = v.secondPhiFace();
}

void
AsmBlock::addResidual(NumericVector<Number> & residual)
{
  for (unsigned int vn = 0; vn < _sys.nVariables(); ++vn)
  {
    MooseVariable & var = _sys.getVariable(_tid, vn);

    if (var.dofIndices().size() > 0)
    {
      _dof_map.constrain_element_vector(_sub_Re[vn], var.dofIndices(), false);

      Real scaling_factor = var.scalingFactor();
      if (scaling_factor != 1.0)
      {
        DenseVector<Number> re(_sub_Re[vn]);
        re.scale(scaling_factor);
        residual.add_vector(re, var.dofIndices());
      }
      else
        residual.add_vector(_sub_Re[vn], var.dofIndices());
    }
  }
}

void
AsmBlock::addJacobian(SparseMatrix<Number> & jacobian)
{
  for (unsigned int vi = 0; vi < _sys.nVariables(); ++vi)
  {
    for (unsigned int vj = 0; vj < _sys.nVariables(); ++vj)
    {
      if ((*_cm)(vi, vj) != 0)
      {
        MooseVariable & ivar = _sys.getVariable(_tid, vi);
        MooseVariable & jvar = _sys.getVariable(_tid, vj);

        if ((ivar.dofIndices().size() > 0) && (jvar.dofIndices().size() > 0))
        {
          _dof_map.constrain_element_matrix(_sub_Ke[vi][vj], ivar.dofIndices(), jvar.dofIndices(), false);

          Real scaling_factor = ivar.scalingFactor();
          if (scaling_factor != 1.0)
          {
            DenseMatrix<Number> ke(_sub_Ke[vi][vj]);
            ke.scale(scaling_factor);
            jacobian.add_matrix(ke, ivar.dofIndices(), jvar.dofIndices());
          }
          else
            jacobian.add_matrix(_sub_Ke[vi][vj], ivar.dofIndices(), jvar.dofIndices());
        }
      }
    }
  }
}

void
AsmBlock::addJacobianBlock(SparseMatrix<Number> & jacobian, unsigned int ivar, unsigned int jvar, const DofMap & dof_map, std::vector<unsigned int> & dof_indices)
{
  DenseMatrix<Number> & ke = jacobianBlock(ivar, jvar);

  // stick it into the matrix
  dof_map.constrain_element_matrix(ke, dof_indices, false);

  Real scaling_factor = _sys.getVariable(_tid, ivar).scalingFactor();
  if (scaling_factor != 1.0)
  {
    DenseMatrix<Number> scaled_ke(ke);
    scaled_ke.scale(scaling_factor);
    jacobian.add_matrix(scaled_ke, dof_indices);
  }
  else
    jacobian.add_matrix(ke, dof_indices);
}
