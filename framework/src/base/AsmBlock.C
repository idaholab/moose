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

DenseMatrix<Number> &
AsmBlock::jacobianBlockNeighbor(Moose::DGJacobianType type, unsigned int ivar, unsigned int jvar)
{
  switch (type)
  {
  case Moose::ElementElement: return _sub_Kee[ivar][jvar];
  case Moose::ElementNeighbor: return _sub_Ken[ivar][jvar];
  case Moose::NeighborElement: return _sub_Kne[ivar][jvar];
  case Moose::NeighborNeighbor: return _sub_Knn[ivar][jvar];
  }
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
  _sub_Rn.resize(n_vars);

  _sub_Kee.resize(n_vars);
  _sub_Ken.resize(n_vars);
  _sub_Kne.resize(n_vars);
  _sub_Knn.resize(n_vars);
  for (unsigned int i = 0; i < n_vars; ++i)
  {
    _sub_Kee[i].resize(n_vars);
    _sub_Ken[i].resize(n_vars);
    _sub_Kne[i].resize(n_vars);
    _sub_Knn[i].resize(n_vars);
  }
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

    _sub_Kee[vi][vj].resize(ivar.dofIndices().size(), jvar.dofIndices().size());
    _sub_Kee[vi][vj].zero();
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
AsmBlock::prepareNeighbor()
{
  for (std::vector<std::pair<unsigned int, unsigned int> >::iterator it = _cm_entry.begin(); it != _cm_entry.end(); ++it)
  {
    unsigned int vi = (*it).first;
    unsigned int vj = (*it).second;

    MooseVariable & ivar = _sys.getVariable(_tid, vi);
    MooseVariable & jvar = _sys.getVariable(_tid, vj);

    _sub_Ken[vi][vj].resize(ivar.dofIndices().size(), jvar.dofIndicesNeighbor().size());
    _sub_Ken[vi][vj].zero();

    _sub_Kne[vi][vj].resize(ivar.dofIndicesNeighbor().size(), jvar.dofIndices().size());
    _sub_Kne[vi][vj].zero();

    _sub_Knn[vi][vj].resize(ivar.dofIndicesNeighbor().size(), jvar.dofIndicesNeighbor().size());
    _sub_Knn[vi][vj].zero();
  }

  unsigned int n_vars = _sys.nVariables();
  for (unsigned int vi = 0; vi < n_vars; vi++)
  {
    MooseVariable & ivar = _sys.getVariable(_tid, vi);
    _sub_Rn[vi].resize(ivar.dofIndices().size());
    _sub_Rn[vi].zero();
  }
}

void
AsmBlock::prepareBlock(unsigned int ivar, unsigned jvar, const std::vector<unsigned int> & dof_indices)
{
  _sub_Kee[ivar][jvar].resize(dof_indices.size(), dof_indices.size());
  _sub_Kee[ivar][jvar].zero();

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
AsmBlock::copyNeighborShapes(unsigned int var)
{
  MooseVariable & v = _sys.getVariable(_tid, var);

  _phi_face_neighbor = v.phiFaceNeighbor();
  _grad_phi_face_neighbor = v.gradPhiFaceNeighbor();
  _second_phi_face_neighbor = v.secondPhiFaceNeighbor();
}

void
AsmBlock::addResidualBlock(NumericVector<Number> & residual, DenseVector<Number> & res_block, std::vector<unsigned int> & dof_indices, Real scaling_factor)
{
  if (dof_indices.size() > 0)
  {
    _dof_map.constrain_element_vector(res_block, dof_indices, false);

    if (scaling_factor != 1.0)
    {
      DenseVector<Number> re(res_block);
      re.scale(scaling_factor);
      residual.add_vector(re, dof_indices);
    }
    else
      residual.add_vector(res_block, dof_indices);
  }
}

void
AsmBlock::addResidual(NumericVector<Number> & residual)
{
  for (unsigned int vn = 0; vn < _sys.nVariables(); ++vn)
  {
    MooseVariable & var = _sys.getVariable(_tid, vn);
    addResidualBlock(residual, _sub_Re[vn], var.dofIndices(), var.scalingFactor());
  }
}

void
AsmBlock::addResidualNeighbor(NumericVector<Number> & residual)
{
  for (unsigned int vn = 0; vn < _sys.nVariables(); ++vn)
  {
    MooseVariable & var = _sys.getVariable(_tid, vn);
    addResidualBlock(residual, _sub_Rn[vn], var.dofIndicesNeighbor(), var.scalingFactor());
  }
}


void
AsmBlock::addJacobianBlock(SparseMatrix<Number> & jacobian, DenseMatrix<Number> & jac_block, std::vector<unsigned int> & idof_indices, std::vector<unsigned int> & jdof_indices, Real scaling_factor)
{
  if ((idof_indices.size() > 0) && (jdof_indices.size() > 0))
  {
    _dof_map.constrain_element_matrix(jac_block, idof_indices, jdof_indices, false);

    if (scaling_factor != 1.0)
    {
      DenseMatrix<Number> ke(jac_block);
      ke.scale(scaling_factor);
      jacobian.add_matrix(ke, idof_indices, jdof_indices);
    }
    else
      jacobian.add_matrix(jac_block, idof_indices, jdof_indices);
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

        addJacobianBlock(jacobian, _sub_Kee[vi][vj], ivar.dofIndices(), jvar.dofIndices(), ivar.scalingFactor());
      }
    }
  }
}

void
AsmBlock::addJacobianNeighbor(SparseMatrix<Number> & jacobian)
{
  for (unsigned int vi = 0; vi < _sys.nVariables(); ++vi)
  {
    for (unsigned int vj = 0; vj < _sys.nVariables(); ++vj)
    {
      if ((*_cm)(vi, vj) != 0)
      {
        MooseVariable & ivar = _sys.getVariable(_tid, vi);
        MooseVariable & jvar = _sys.getVariable(_tid, vj);

        addJacobianBlock(jacobian, _sub_Ken[vi][vj], ivar.dofIndices(), jvar.dofIndicesNeighbor(), ivar.scalingFactor());
        addJacobianBlock(jacobian, _sub_Kne[vi][vj], ivar.dofIndicesNeighbor(), jvar.dofIndices(), ivar.scalingFactor());
        addJacobianBlock(jacobian, _sub_Knn[vi][vj], ivar.dofIndicesNeighbor(), jvar.dofIndicesNeighbor(), ivar.scalingFactor());
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
