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
    _tid(tid),
    _max_cached_residuals(0),
    _max_cached_jacobians(0)
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
  default:
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
    _sub_Rn[vi].resize(ivar.dofIndicesNeighbor().size());
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
    std::vector<unsigned int> di(dof_indices);
    _dof_map.constrain_element_vector(res_block, di, false);

    if (scaling_factor != 1.0)
    {
      DenseVector<Number> re(res_block);
      re.scale(scaling_factor);
      residual.add_vector(re, di);
    }
    else
    {
//      for(unsigned int i=0; i<dof_indices.size(); i++)
//      {
//        std::cout<<"Dof: "<<dof_indices[i]<<" "<<res_block(i)<<std::endl;
//      }
/*
      std::cout<<residual<<std::endl;
      residual.close();
*/
      residual.add_vector(res_block, di);
//      residual.close();
//      std::cout<<residual<<std::endl;

    }
  }
}

void
AsmBlock::cacheResidualBlock(DenseVector<Number> & res_block, std::vector<unsigned int> & dof_indices, Real scaling_factor)
{
  if (dof_indices.size() > 0)
  {
    std::vector<unsigned int> di(dof_indices);
    _dof_map.constrain_element_vector(res_block, di, false);

    if (scaling_factor != 1.0)
    {
      DenseVector<Number> re(res_block);
      re.scale(scaling_factor);

      for(unsigned int i=0; i<re.size(); i++)
      {
        _cached_residual_values.push_back(re(i));
        _cached_residual_rows.push_back(di[i]);
      }
    }
    else
    {
      for(unsigned int i=0; i<res_block.size(); i++)
      {
        _cached_residual_values.push_back(res_block(i));
        _cached_residual_rows.push_back(di[i]);
      }
    }
  }

  res_block.zero();
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
AsmBlock::cacheResidual()
{
  for (unsigned int vn = 0; vn < _sys.nVariables(); ++vn)
  {
    MooseVariable & var = _sys.getVariable(_tid, vn);
    cacheResidualBlock(_sub_Re[vn], var.dofIndices(), var.scalingFactor());
  }
}

void
AsmBlock::cacheResidualNeighbor()
{
  for (unsigned int vn = 0; vn < _sys.nVariables(); ++vn)
  {
    MooseVariable & var = _sys.getVariable(_tid, vn);

    cacheResidualBlock(_sub_Rn[vn], var.dofIndicesNeighbor(), var.scalingFactor());
  }
}


void
AsmBlock::addCachedResidual(NumericVector<Number> & residual)
{
  mooseAssert(_cached_residual_values.size() == _cached_residual_rows.size(), "Number of cached residuals and number of rows must match!");

//  residual.add_vector(_cached_residual_values, _cached_residual_rows);

//  residual.print();

  for(unsigned int i=0; i<_cached_residual_values.size(); i++)
  {
//    std::cerr<<_cached_residual_rows[i]<<std::endl;

    residual.add(_cached_residual_rows[i], _cached_residual_values[i]);
  }

  if(_max_cached_residuals < _cached_residual_values.size())
    _max_cached_residuals = _cached_residual_values.size();

  // Try to be more efficient from now on
  // The 2 is just a fudge factor to keep us from having to grow the vector during assembly
  _cached_residual_values.clear();
  _cached_residual_values.reserve(_max_cached_residuals*2);

  _cached_residual_rows.clear();
  _cached_residual_rows.reserve(_max_cached_residuals*2);
}


void
AsmBlock::setResidualBlock(NumericVector<Number> & residual, DenseVector<Number> & res_block, std::vector<unsigned int> & dof_indices, Real scaling_factor)
{
  if (dof_indices.size() > 0)
  {
    std::vector<unsigned int> di(dof_indices);
    _dof_map.constrain_element_vector(res_block, di, false);

    if (scaling_factor != 1.0)
    {
      DenseVector<Number> re(res_block);
      re.scale(scaling_factor);
      for(unsigned int i=0; i<di.size(); i++)
        residual.set(di[i], res_block(i));
    }
    else
      for(unsigned int i=0; i<di.size(); i++)
        residual.set(di[i], res_block(i));
  }
}

void
AsmBlock::setResidual(NumericVector<Number> & residual)
{
  for (unsigned int vn = 0; vn < _sys.nVariables(); ++vn)
  {
    MooseVariable & var = _sys.getVariable(_tid, vn);
    setResidualBlock(residual, _sub_Re[vn], var.dofIndices(), var.scalingFactor());
  }
}

void
AsmBlock::setResidualNeighbor(NumericVector<Number> & residual)
{
  for (unsigned int vn = 0; vn < _sys.nVariables(); ++vn)
  {
    MooseVariable & var = _sys.getVariable(_tid, vn);
    setResidualBlock(residual, _sub_Rn[vn], var.dofIndicesNeighbor(), var.scalingFactor());
  }
}


void
AsmBlock::addJacobianBlock(SparseMatrix<Number> & jacobian, DenseMatrix<Number> & jac_block, std::vector<unsigned int> & idof_indices, std::vector<unsigned int> & jdof_indices, Real scaling_factor)
{
  if ((idof_indices.size() > 0) && (jdof_indices.size() > 0))
  {
    std::vector<unsigned int> di(idof_indices);
    std::vector<unsigned int> dj(jdof_indices);
    _dof_map.constrain_element_matrix(jac_block, di, dj, false);

    if (scaling_factor != 1.0)
    {
      DenseMatrix<Number> ke(jac_block);
      ke.scale(scaling_factor);
      jacobian.add_matrix(ke, di, dj);
    }
    else
      jacobian.add_matrix(jac_block, di, dj);
  }
}

void
AsmBlock::cacheJacobianBlock(DenseMatrix<Number> & jac_block, std::vector<unsigned int> & idof_indices, std::vector<unsigned int> & jdof_indices, Real scaling_factor)
{
  if ((idof_indices.size() > 0) && (jdof_indices.size() > 0))
  {
    std::vector<unsigned int> di(idof_indices);
    std::vector<unsigned int> dj(jdof_indices);
    _dof_map.constrain_element_matrix(jac_block, di, dj, false);

    if (scaling_factor != 1.0)
    {
      DenseMatrix<Number> ke(jac_block);
      ke.scale(scaling_factor);

      for(unsigned int i=0; i<di.size(); i++)
        for(unsigned int j=0; j<dj.size(); j++)
        {
          _cached_jacobian_values.push_back(ke(i, j));
          _cached_jacobian_rows.push_back(di[i]);
          _cached_jacobian_cols.push_back(dj[j]);
        }
    }
    else
    {
      for(unsigned int i=0; i<di.size(); i++)
        for(unsigned int j=0; j<dj.size(); j++)
        {
          _cached_jacobian_values.push_back(jac_block(i, j));
          _cached_jacobian_rows.push_back(di[i]);
          _cached_jacobian_cols.push_back(dj[j]);
        }
    }
  }

  jac_block.zero();
}


void
AsmBlock::addCachedJacobian(SparseMatrix<Number> & jacobian)
{
  mooseAssert(_cached_jacobian_rows.size() == _cached_jacobian_cols.size(),
              "Error: Cached data sizes MUST be the same!");

  for(unsigned int i=0; i<_cached_jacobian_rows.size(); i++)
    jacobian.add(_cached_jacobian_rows[i], _cached_jacobian_cols[i], _cached_jacobian_values[i]);

  if(_max_cached_jacobians < _cached_jacobian_values.size())
    _max_cached_jacobians = _cached_jacobian_values.size();

  // Try to be more efficient from now on
  // The 2 is just a fudge factor to keep us from having to grow the vector during assembly
  _cached_jacobian_values.clear();
  _cached_jacobian_values.reserve(_max_cached_jacobians*2);

  _cached_jacobian_rows.clear();
  _cached_jacobian_rows.reserve(_max_cached_jacobians*2);

  _cached_jacobian_cols.clear();
  _cached_jacobian_cols.reserve(_max_cached_jacobians*2);
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
AsmBlock::cacheJacobian()
{
  for (unsigned int vi = 0; vi < _sys.nVariables(); ++vi)
  {
    for (unsigned int vj = 0; vj < _sys.nVariables(); ++vj)
    {
      if ((*_cm)(vi, vj) != 0)
      {
        MooseVariable & ivar = _sys.getVariable(_tid, vi);
        MooseVariable & jvar = _sys.getVariable(_tid, vj);

        cacheJacobianBlock(_sub_Kee[vi][vj], ivar.dofIndices(), jvar.dofIndices(), ivar.scalingFactor());
      }
    }
  }
}

void
AsmBlock::cacheJacobianNeighbor()
{
  for (unsigned int vi = 0; vi < _sys.nVariables(); ++vi)
  {
    for (unsigned int vj = 0; vj < _sys.nVariables(); ++vj)
    {
      if ((*_cm)(vi, vj) != 0)
      {
        MooseVariable & ivar = _sys.getVariable(_tid, vi);
        MooseVariable & jvar = _sys.getVariable(_tid, vj);

        cacheJacobianBlock(_sub_Ken[vi][vj], ivar.dofIndices(), jvar.dofIndicesNeighbor(), ivar.scalingFactor());
        cacheJacobianBlock(_sub_Kne[vi][vj], ivar.dofIndicesNeighbor(), jvar.dofIndices(), ivar.scalingFactor());
        cacheJacobianBlock(_sub_Knn[vi][vj], ivar.dofIndicesNeighbor(), jvar.dofIndicesNeighbor(), ivar.scalingFactor());
      }
    }
  }
}

void
AsmBlock::addJacobianBlock(SparseMatrix<Number> & jacobian, unsigned int ivar, unsigned int jvar, const DofMap & dof_map, std::vector<unsigned int> & dof_indices)
{
  DenseMatrix<Number> & ke = jacobianBlock(ivar, jvar);

  // stick it into the matrix
  std::vector<unsigned int> di(dof_indices);
  dof_map.constrain_element_matrix(ke, di, false);

  Real scaling_factor = _sys.getVariable(_tid, ivar).scalingFactor();
  if (scaling_factor != 1.0)
  {
    DenseMatrix<Number> scaled_ke(ke);
    scaled_ke.scale(scaling_factor);
    jacobian.add_matrix(scaled_ke, di);
  }
  else
    jacobian.add_matrix(ke, di);
}

void
AsmBlock::addJacobianNeighbor(SparseMatrix<Number> & jacobian, unsigned int ivar, unsigned int jvar, const DofMap & dof_map, std::vector<unsigned int> & dof_indices, std::vector<unsigned int> & neighbor_dof_indices)
{
  DenseMatrix<Number> & kee = jacobianBlock(ivar, jvar);
  DenseMatrix<Number> & ken = jacobianBlockNeighbor(Moose::ElementNeighbor, ivar, jvar);
  DenseMatrix<Number> & kne = jacobianBlockNeighbor(Moose::NeighborElement, ivar, jvar);
  DenseMatrix<Number> & knn = jacobianBlockNeighbor(Moose::NeighborNeighbor, ivar, jvar);

  std::vector<unsigned int> di(dof_indices);
  std::vector<unsigned int> dn(neighbor_dof_indices);
  // stick it into the matrix
  dof_map.constrain_element_matrix(kee, di, false);
  dof_map.constrain_element_matrix(ken, di, dn, false);
  dof_map.constrain_element_matrix(kne, dn, di, false);
  dof_map.constrain_element_matrix(knn, dn, false);

  Real scaling_factor = _sys.getVariable(_tid, ivar).scalingFactor();
  if (scaling_factor != 1.0)
  {
    {
      DenseMatrix<Number> scaled_ke(kee);
      scaled_ke.scale(scaling_factor);
      jacobian.add_matrix(scaled_ke, di);
    }
    {
      DenseMatrix<Number> scaled_ke(ken);
      scaled_ke.scale(scaling_factor);
      jacobian.add_matrix(scaled_ke, di, dn);
    }
    {
      DenseMatrix<Number> scaled_ke(kne);
      scaled_ke.scale(scaling_factor);
      jacobian.add_matrix(scaled_ke, dn, di);
    }
    {
      DenseMatrix<Number> scaled_ke(knn);
      scaled_ke.scale(scaling_factor);
      jacobian.add_matrix(scaled_ke, dn);
    }
  }
  else
  {
    jacobian.add_matrix(kee, di);
    jacobian.add_matrix(ken, di, dn);
    jacobian.add_matrix(kne, dn, di);
    jacobian.add_matrix(knn, dn);
  }
}
