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

#include "Assembly.h"

// MOOSE includes
#include "SubProblem.h"
#include "ArbitraryQuadrature.h"
#include "SystemBase.h"
// libMesh
#include "quadrature_gauss.h"
#include "fe_interface.h"


Assembly::Assembly(SystemBase & sys, CouplingMatrix * & cm, THREAD_ID tid) :
    _sys(sys),
    _cm(cm),
    _dof_map(_sys.dofMap()),
    _tid(tid),
    _mesh(sys.mesh()),
    _mesh_dimension(_mesh.dimension()),
    _current_qrule(NULL),
    _current_qrule_volume(NULL),
    _current_qrule_arbitrary(NULL),
    _current_qrule_face(NULL),
    _current_qface_arbitrary(NULL),

    _current_elem(NULL),
    _current_side(0),
    _current_side_elem(NULL),
    _current_neighbor_elem(NULL),
    _current_node(NULL),
    _current_neighbor_node(NULL),

    _max_cached_residuals(0),
    _max_cached_jacobians(0)
{
  // Build fe's for the helpers
  getFE(FEType(FIRST, LAGRANGE));
  getFEFace(FEType(FIRST, LAGRANGE));

  // Build an FE helper object for this type for each dimension up to the dimension of the current mesh
  for(unsigned int dim=1; dim<=_mesh_dimension; dim++)
  {
    _holder_fe_helper[dim] = &_fe[dim][FEType(FIRST, LAGRANGE)];
    (*_holder_fe_helper[dim])->get_xyz();
    (*_holder_fe_helper[dim])->get_JxW();

    _holder_fe_face_helper[dim] = &_fe_face[dim][FEType(FIRST, LAGRANGE)];
    (*_holder_fe_face_helper[dim])->get_xyz();
    (*_holder_fe_face_helper[dim])->get_JxW();
    (*_holder_fe_face_helper[dim])->get_normals();
  }
}

Assembly::~Assembly()
{
  for(unsigned int dim=1; dim<=_mesh_dimension; dim++)
    for (std::map<FEType, FEBase *>::iterator it = _fe[dim].begin(); it != _fe[dim].end(); ++it)
      delete it->second;
  for(unsigned int dim=1; dim<=_mesh_dimension; dim++)
    for (std::map<FEType, FEBase *>::iterator it = _fe_face[dim].begin(); it != _fe_face[dim].end(); ++it)
      delete it->second;
  for(unsigned int dim=1; dim<=_mesh_dimension; dim++)
    for (std::map<FEType, FEBase *>::iterator it = _fe_neighbor[dim].begin(); it != _fe_neighbor[dim].end(); ++it)
      delete it->second;

  for (std::map<unsigned int, QBase *>::iterator it = _holder_qrule_volume.begin(); it != _holder_qrule_volume.end(); ++it)
    delete it->second;
  for (std::map<unsigned int, ArbitraryQuadrature *>::iterator it = _holder_qrule_arbitrary.begin(); it != _holder_qrule_arbitrary.end(); ++it)
    delete it->second;
  for (std::map<unsigned int, ArbitraryQuadrature *>::iterator it = _holder_qface_arbitrary.begin(); it != _holder_qface_arbitrary.end(); ++it)
    delete it->second;
  for (std::map<unsigned int, QBase *>::iterator it = _holder_qrule_face.begin(); it != _holder_qrule_face.end(); ++it)
    delete it->second;

  delete _current_side_elem;
}

FEBase * &
Assembly::getFE(FEType type)
{
  // Build an FE object for this type for each dimension up to the dimension of the current mesh
  for(unsigned int dim=1; dim<=_mesh_dimension; dim++)
  {
    if (!_fe[dim][type])
      _fe[dim][type] = FEBase::build(dim, type).release();
  }

  return _current_fe[type];
}

FEBase * &
Assembly::getFEFace(FEType type)
{
  // Build an FE object for this type for each dimension up to the dimension of the current mesh
  for(unsigned int dim=1; dim<=_mesh_dimension; dim++)
  {
    if (!_fe_face[dim][type])
      _fe_face[dim][type] = FEBase::build(dim, type).release();
  }

  return _current_fe_face[type];
}

FEBase * &
Assembly::getFEFaceNeighbor(FEType type)
{
  // Build an FE object for this type for each dimension up to the dimension of the current mesh
  for(unsigned int dim=1; dim<=_mesh_dimension; dim++)
  {
    if (!_fe_neighbor[dim][type])
      _fe_neighbor[dim][type] = FEBase::build(_mesh_dimension, type).release();
  }

  return _current_fe_neighbor[type];
}

void
Assembly::createQRules(QuadratureType type, Order o)
{
  _holder_qrule_volume.clear();
  for(unsigned int dim=1; dim<=_mesh_dimension; dim++)
    _holder_qrule_volume[dim] = QBase::build(type, dim, o).release();

  _holder_qrule_face.clear();
  for(unsigned int dim=1; dim<=_mesh_dimension; dim++)
    _holder_qrule_face[dim] = QBase::build(type, dim - 1, o).release();

  _holder_qrule_arbitrary.clear();
  for(unsigned int dim=1; dim<=_mesh_dimension; dim++)
    _holder_qrule_arbitrary[dim] = new ArbitraryQuadrature(dim, o);

//  setVolumeQRule(_qrule_volume);
//  setFaceQRule(_qrule_face);
}

void
Assembly::setVolumeQRule(QBase * qrule, unsigned int dim)
{
  _current_qrule = qrule;

  for (std::map<FEType, FEBase *>::iterator it = _fe[dim].begin(); it != _fe[dim].end(); ++it)
    it->second->attach_quadrature_rule(_current_qrule);
}

void
Assembly::setFaceQRule(QBase * qrule, unsigned int dim)
{
  _current_qrule_face = qrule;

  for (std::map<FEType, FEBase *>::iterator it = _fe_face[dim].begin(); it != _fe_face[dim].end(); ++it)
    it->second->attach_quadrature_rule(_current_qrule_face);
}

void
Assembly::reinitFE(const Elem * elem)
{
  unsigned int dim = elem->dim();

  for (std::map<FEType, FEBase *>::iterator it = _fe[dim].begin(); it != _fe[dim].end(); ++it)
  {
    FEBase * fe = it->second;
    const FEType & fe_type = it->first;
    fe->reinit(elem);
    _current_fe[fe_type] = fe;

    _fe_phi[fe_type] = fe->get_phi();
    _fe_grad_phi[fe_type] = fe->get_dphi();
    if(_need_second_derivative[fe_type])
      _fe_second_phi[fe_type] = fe->get_d2phi();
  }

  // During that last loop the helper objects will have been reinitialized as well
  // We need to dig out the q_points and JxW from it.
  _current_q_points = (*_holder_fe_helper[dim])->get_xyz();
  _current_JxW = (*_holder_fe_helper[dim])->get_JxW();
}

void
Assembly::reinitFEFace(const Elem * elem, unsigned int side)
{
  unsigned int dim = elem->dim();

  for (std::map<FEType, FEBase *>::iterator it = _fe_face[dim].begin(); it != _fe_face[dim].end(); ++it)
  {
    FEBase * fe_face = it->second;
    const FEType & fe_type = it->first;
    fe_face->reinit(elem, side);
    _current_fe_face[it->first] = fe_face;

    _fe_phi_face[fe_type] = fe_face->get_phi();
    _fe_grad_phi_face[fe_type] = fe_face->get_dphi();
    if(_need_second_derivative[fe_type])
      _fe_second_phi_face[fe_type] = fe_face->get_d2phi();
  }

  // During that last loop the helper objects will have been reinitialized as well
  // We need to dig out the q_points and JxW from it.
  _current_q_points_face = (*_holder_fe_face_helper[dim])->get_xyz();
  _current_JxW_face = (*_holder_fe_face_helper[dim])->get_JxW();
  _current_normals = (*_holder_fe_face_helper[dim])->get_normals();
}

void
Assembly::reinit(const Elem * elem)
{
  _current_elem = elem;
  unsigned int elem_dimension = elem->dim();

  _current_qrule_volume = _holder_qrule_volume[elem_dimension];

  // Make sure the qrule is the right one
  if(_current_qrule != _current_qrule_volume)
    setVolumeQRule(_current_qrule_volume, elem_dimension);

  reinitFE(elem);

  // set the coord transformation
  _coord.resize(_current_qrule->n_points());
  switch (_sys.subproblem().coordSystem())
  {
  case Moose::COORD_XYZ:
    for (unsigned int qp = 0; qp < _current_qrule->n_points(); qp++)
      _coord[qp] = 1.;
    break;

  case Moose::COORD_RZ:
    for (unsigned int qp = 0; qp < _current_qrule->n_points(); qp++)
      _coord[qp] = 2 * M_PI * _current_q_points[qp](0);
    break;

  default:
    mooseError("Unknown coordinate system");
    break;
  }

  //Compute the area of the element
  _current_elem_volume = 0.;
  for (unsigned int qp = 0; qp < _current_qrule->n_points(); qp++)
    _current_elem_volume += _current_JxW[qp] * _coord[qp];
}

void
Assembly::reinitAtPhysical(const Elem * elem, const std::vector<Point> & physical_points)
{
  std::vector<Point> reference_points;

  unsigned int elem_dimension = elem->dim();

  FEInterface::inverse_map(elem->dim(), FEType(), elem, physical_points, reference_points);

  reinit(elem, reference_points);

  // Save off the physical points
  _current_physical_points = physical_points;
}

void
Assembly::reinit(const Elem * elem, const std::vector<Point> & reference_points)
{
  _current_elem = elem;

  unsigned int elem_dimension = _current_elem->dim();

  _current_qrule_arbitrary = _holder_qrule_arbitrary[elem_dimension];

  // Make sure the qrule is the right one
  if(_current_qrule != _current_qrule_arbitrary)
    setVolumeQRule(_current_qrule_arbitrary, elem_dimension);

  _current_qrule_arbitrary->setPoints(reference_points);

  reinitFE(elem);
}

void
Assembly::reinit(const Elem * elem, unsigned int side)
{
  _current_elem = elem;
  _current_side = side;

  unsigned int elem_dimension = _current_elem->dim();

  if(_current_qrule_face != _holder_qrule_face[elem_dimension])
  {
    _current_qrule_face = _holder_qrule_face[elem_dimension];
    setFaceQRule(_current_qrule_face, elem_dimension);
  }

  if (_current_side_elem)
    delete _current_side_elem;
  _current_side_elem = elem->build_side(side).release();

  reinitFEFace(elem, side);

  // set the coord transformation
  _coord.resize(_current_qrule_face->n_points());
  switch (_sys.subproblem().coordSystem())
  {
  case Moose::COORD_XYZ:
    for (unsigned int qp = 0; qp < _current_qrule_face->n_points(); qp++)
      _coord[qp] = 1.;
    break;

  case Moose::COORD_RZ:
    for (unsigned int qp = 0; qp < _current_qrule_face->n_points(); qp++)
      _coord[qp] = 2 * M_PI * _current_q_points_face[qp](0);
    break;

  default:
    mooseError("Unknown coordinate system");
    break;
  }

  //Compute the area of the element
  _current_side_volume = 0.;
  for (unsigned int qp = 0; qp < _current_qrule_face->n_points(); qp++)
    _current_side_volume += _current_JxW_face[qp] * _coord[qp];
}

void
Assembly::reinit(const Node * node)
{
  _current_node = node;
}

void
Assembly::reinitNodeNeighbor(const Node * node)
{
  _current_neighbor_node = node;
}

void
Assembly::reinit(const Elem * elem, unsigned int side, const Elem * neighbor)
{
  // reinit this element
  reinit(elem, side);

  unsigned int neighbor_dim = neighbor->dim();

  // reinit neighbor element
  for (std::map<FEType, FEBase *>::iterator it = _fe_neighbor[neighbor_dim].begin(); it != _fe_neighbor[neighbor_dim].end(); ++it)
  {
    FEBase * fe_neighbor = it->second;
    const FEType & fe_type = it->first;

    // Find locations of quadrature points on the neighbor
    std::vector<Point> qface_neighbor_point;
    libMesh::FEInterface::inverse_map (neighbor_dim, fe_type, neighbor, _current_q_points_face, qface_neighbor_point);
    // Calculate the neighbor element shape functions at those locations
    it->second->reinit(neighbor, &qface_neighbor_point);

    _current_fe_neighbor[fe_type] = fe_neighbor;

    _fe_phi_face_neighbor[fe_type] = fe_neighbor->get_phi();
    _fe_grad_phi_face_neighbor[fe_type] = fe_neighbor->get_dphi();
    if(_need_second_derivative[fe_type])
      _fe_second_phi_face_neighbor[fe_type] = fe_neighbor->get_d2phi();
  }

  _current_neighbor_elem = neighbor;
}

void
Assembly::reinitNeighborAtPhysical(const Elem * neighbor, unsigned int /*neighbor_side*/, const std::vector<Point> & physical_points)
{
  std::vector<Point> reference_points;

  unsigned int neighbor_dim = neighbor->dim();

  // reinit neighbor element
  for (std::map<FEType, FEBase *>::iterator it = _fe_neighbor[neighbor_dim].begin(); it != _fe_neighbor[neighbor_dim].end(); ++it)
  {
    FEBase * fe_neighbor = it->second;
    FEType fe_type = it->first;

    FEInterface::inverse_map(neighbor_dim, fe_type, neighbor, physical_points, reference_points);

    it->second->reinit(neighbor, &reference_points);

    _current_fe_neighbor[it->first] = it->second;

    _fe_phi_face_neighbor[fe_type] = fe_neighbor->get_phi();
    _fe_grad_phi_face_neighbor[fe_type] = fe_neighbor->get_dphi();
    if(_need_second_derivative[fe_type])
      _fe_second_phi_face_neighbor[fe_type] = fe_neighbor->get_d2phi();
  }

  // Save off the physical points
  _current_physical_points = physical_points;

  _current_qrule_arbitrary = _holder_qrule_arbitrary[neighbor_dim];

  // Make sure the qrule is the right one
  if(_current_qrule != _current_qrule_arbitrary)
    setVolumeQRule(_current_qrule_arbitrary, neighbor_dim);

  _current_qrule_arbitrary->setPoints(reference_points);
  _current_neighbor_elem = neighbor;
}

DenseMatrix<Number> &
Assembly::jacobianBlockNeighbor(Moose::DGJacobianType type, unsigned int ivar, unsigned int jvar)
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
Assembly::init()
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

  unsigned int n_scalar_vars = _sys.nScalarVariables();
  _scalar_Re.resize(n_scalar_vars);

  _scalar_Kee.resize(n_scalar_vars);
  _scalar_Ken.resize(n_scalar_vars);
  _scalar_Kne.resize(n_vars);

  for (unsigned int i = 0; i < n_scalar_vars; ++i)
  {
    _scalar_Kee[i].resize(n_scalar_vars);
    _scalar_Ken[i].resize(n_vars);
  }

  for (unsigned int i = 0; i < n_vars; ++i)
    _scalar_Kne[i].resize(n_scalar_vars);
}

void
Assembly::prepare()
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
Assembly::prepareNeighbor()
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
Assembly::prepareBlock(unsigned int ivar, unsigned jvar, const std::vector<unsigned int> & dof_indices)
{
  _sub_Kee[ivar][jvar].resize(dof_indices.size(), dof_indices.size());
  _sub_Kee[ivar][jvar].zero();

  _sub_Re[ivar].resize(dof_indices.size());
  _sub_Re[ivar].zero();
}

void
Assembly::prepareScalar()
{
  _scalar_has_off_diag_contributions = false;

  unsigned int n_scalar_vars = _sys.nScalarVariables();

  for (unsigned int vi = 0; vi < n_scalar_vars; vi++)
  {
    MooseVariableScalar & ivar = _sys.getScalarVariable(_tid, vi);
    unsigned int lm_dofs = ivar.dofIndices().size();

    // LM residual/jacobian blocks
    _scalar_Re[vi].resize(lm_dofs);
    _scalar_Re[vi].zero();

    for (unsigned int vj = 0; vj < n_scalar_vars; vj++)
    {
      MooseVariableScalar & jvar = _sys.getScalarVariable(_tid, vj);
      unsigned int jlm_dofs = jvar.dofIndices().size();

      _scalar_Kee[vi][vj].resize(lm_dofs, jlm_dofs);
      _scalar_Kee[vi][vj].zero();
    }
  }
}

void
Assembly::prepareOffDiagScalar()
{
  _scalar_has_off_diag_contributions = true;

  unsigned int n_vars = _sys.nVariables();
  unsigned int n_scalar_vars = _sys.nScalarVariables();

  for (unsigned int vj = 0; vj < n_vars; vj++)
  {
    MooseVariable & jvar = _sys.getVariable(_tid, vj);
    unsigned int ced_dofs = jvar.dofIndices().size();

    _sub_Re[vj].resize(ced_dofs);
    _sub_Re[vj].zero();
  }

  for (unsigned int vi = 0; vi < n_scalar_vars; vi++)
  {
    MooseVariableScalar & ivar = _sys.getScalarVariable(_tid, vi);
    unsigned int lm_dofs = ivar.dofIndices().size();

    for (unsigned int vj = 0; vj < n_vars; vj++)
    {
      MooseVariable & jvar = _sys.getVariable(_tid, vj);
      unsigned int ced_dofs = jvar.dofIndices().size();

      _scalar_Ken[vi][vj].resize(lm_dofs, ced_dofs);
      _scalar_Ken[vi][vj].zero();

      _scalar_Kne[vj][vi].resize(ced_dofs, lm_dofs);
      _scalar_Kne[vj][vi].zero();
    }
  }
}

void
Assembly::copyShapes(unsigned int var)
{
  MooseVariable & v = _sys.getVariable(_tid, var);

  _phi = v.phi();
  _grad_phi = v.gradPhi();
  _second_phi = v.secondPhi();
}

void
Assembly::copyFaceShapes(unsigned int var)
{
  MooseVariable & v = _sys.getVariable(_tid, var);

  _phi_face = v.phiFace();
  _grad_phi_face = v.gradPhiFace();
  _second_phi_face = v.secondPhiFace();
}

void
Assembly::copyNeighborShapes(unsigned int var)
{
  MooseVariable & v = _sys.getVariable(_tid, var);

  if(v.usesPhi())
    _phi_face_neighbor = v.phiFaceNeighbor();
  if(v.usesGradPhi())
    _grad_phi_face_neighbor = v.gradPhiFaceNeighbor();
  if(v.usesSecondPhi())
    _second_phi_face_neighbor = v.secondPhiFaceNeighbor();
}

void
Assembly::addResidualBlock(NumericVector<Number> & residual, DenseVector<Number> & res_block, const std::vector<unsigned int> & dof_indices, Real scaling_factor)
{
  if (dof_indices.size() > 0)
  {
    std::vector<unsigned int> di(dof_indices);
    _dof_map.constrain_element_vector(res_block, di, false);

    if (scaling_factor != 1.0)
    {
      _tmp_Re = res_block;
      _tmp_Re *= scaling_factor;
      residual.add_vector(_tmp_Re, di);
    }
    else
    {
      residual.add_vector(res_block, di);
    }
  }
}

void
Assembly::cacheResidualBlock(DenseVector<Number> & res_block, std::vector<unsigned int> & dof_indices, Real scaling_factor)
{
  if (dof_indices.size() > 0)
  {
    std::vector<unsigned int> di(dof_indices);
    _dof_map.constrain_element_vector(res_block, di, false);

    if (scaling_factor != 1.0)
    {
      _tmp_Re = res_block;
      _tmp_Re *= scaling_factor;

      for(unsigned int i=0; i<_tmp_Re.size(); i++)
      {
        _cached_residual_values.push_back(_tmp_Re(i));
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
Assembly::addResidual(NumericVector<Number> & residual)
{
  for (unsigned int vn = 0; vn < _sys.nVariables(); ++vn)
  {
    MooseVariable & var = _sys.getVariable(_tid, vn);
    addResidualBlock(residual, _sub_Re[vn], var.dofIndices(), var.scalingFactor());
  }
}

void
Assembly::addResidualNeighbor(NumericVector<Number> & residual)
{
  for (unsigned int vn = 0; vn < _sys.nVariables(); ++vn)
  {
    MooseVariable & var = _sys.getVariable(_tid, vn);
    addResidualBlock(residual, _sub_Rn[vn], var.dofIndicesNeighbor(), var.scalingFactor());
  }
}

void
Assembly::addResidualScalar(NumericVector<Number> & residual)
{
  // add the scalar variables residuals
  for (unsigned int vn = 0; vn < _sys.nScalarVariables(); ++vn)
  {
    MooseVariableScalar & var = _sys.getScalarVariable(_tid, vn);
    addResidualBlock(residual, _scalar_Re[vn], var.dofIndices(), var.scalingFactor());
  }
  if (_scalar_has_off_diag_contributions)
  {
    // add the other variables residuals
    for (unsigned int vn = 0; vn < _sys.nVariables(); ++vn)
    {
      MooseVariable & var = _sys.getVariable(_tid, vn);
      addResidualBlock(residual, _sub_Re[vn], var.dofIndices(), var.scalingFactor());
    }
  }
}


void
Assembly::cacheResidual()
{
  for (unsigned int vn = 0; vn < _sys.nVariables(); ++vn)
  {
    MooseVariable & var = _sys.getVariable(_tid, vn);
    cacheResidualBlock(_sub_Re[vn], var.dofIndices(), var.scalingFactor());
  }
}

void
Assembly::cacheResidualNeighbor()
{
  for (unsigned int vn = 0; vn < _sys.nVariables(); ++vn)
  {
    MooseVariable & var = _sys.getVariable(_tid, vn);

    cacheResidualBlock(_sub_Rn[vn], var.dofIndicesNeighbor(), var.scalingFactor());
  }
}


void
Assembly::addCachedResidual(NumericVector<Number> & residual)
{
  mooseAssert(_cached_residual_values.size() == _cached_residual_rows.size(), "Number of cached residuals and number of rows must match!");

  for(unsigned int i=0; i<_cached_residual_values.size(); i++)
  {
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
Assembly::setResidualBlock(NumericVector<Number> & residual, DenseVector<Number> & res_block, std::vector<unsigned int> & dof_indices, Real scaling_factor)
{
  if (dof_indices.size() > 0)
  {
    std::vector<unsigned int> di(dof_indices);
    _dof_map.constrain_element_vector(res_block, di, false);

    if (scaling_factor != 1.0)
    {
      _tmp_Re = res_block;
      _tmp_Re *= scaling_factor;
      for(unsigned int i=0; i<di.size(); i++)
        residual.set(di[i], _tmp_Re(i));
    }
    else
      for(unsigned int i=0; i<di.size(); i++)
        residual.set(di[i], res_block(i));
  }
}

void
Assembly::setResidual(NumericVector<Number> & residual)
{
  for (unsigned int vn = 0; vn < _sys.nVariables(); ++vn)
  {
    MooseVariable & var = _sys.getVariable(_tid, vn);
    setResidualBlock(residual, _sub_Re[vn], var.dofIndices(), var.scalingFactor());
  }
}

void
Assembly::setResidualNeighbor(NumericVector<Number> & residual)
{
  for (unsigned int vn = 0; vn < _sys.nVariables(); ++vn)
  {
    MooseVariable & var = _sys.getVariable(_tid, vn);
    setResidualBlock(residual, _sub_Rn[vn], var.dofIndicesNeighbor(), var.scalingFactor());
  }
}


void
Assembly::addJacobianBlock(SparseMatrix<Number> & jacobian, DenseMatrix<Number> & jac_block, const std::vector<unsigned int> & idof_indices, const std::vector<unsigned int> & jdof_indices, Real scaling_factor)
{
  if ((idof_indices.size() > 0) && (jdof_indices.size() > 0))
  {
    std::vector<unsigned int> di(idof_indices);
    std::vector<unsigned int> dj(jdof_indices);
    _dof_map.constrain_element_matrix(jac_block, di, dj, false);

    if (scaling_factor != 1.0)
    {
      _tmp_Ke = jac_block;
      _tmp_Ke *= scaling_factor;
      jacobian.add_matrix(_tmp_Ke, di, dj);
    }
    else
      jacobian.add_matrix(jac_block, di, dj);
  }
}

void
Assembly::cacheJacobianBlock(DenseMatrix<Number> & jac_block, std::vector<unsigned int> & idof_indices, std::vector<unsigned int> & jdof_indices, Real scaling_factor)
{
  if ((idof_indices.size() > 0) && (jdof_indices.size() > 0))
  {
    std::vector<unsigned int> di(idof_indices);
    std::vector<unsigned int> dj(jdof_indices);
    _dof_map.constrain_element_matrix(jac_block, di, dj, false);

    if (scaling_factor != 1.0)
    {
      _tmp_Ke = jac_block;
      _tmp_Ke *= scaling_factor;

      for(unsigned int i=0; i<di.size(); i++)
        for(unsigned int j=0; j<dj.size(); j++)
        {
          _cached_jacobian_values.push_back(_tmp_Ke(i, j));
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
Assembly::addCachedJacobian(SparseMatrix<Number> & jacobian)
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
Assembly::addJacobian(SparseMatrix<Number> & jacobian)
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
Assembly::addJacobianNeighbor(SparseMatrix<Number> & jacobian)
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
Assembly::cacheJacobian()
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
Assembly::cacheJacobianNeighbor()
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
Assembly::addJacobianBlock(SparseMatrix<Number> & jacobian, unsigned int ivar, unsigned int jvar, const DofMap & dof_map, std::vector<unsigned int> & dof_indices)
{
  DenseMatrix<Number> & ke = jacobianBlock(ivar, jvar);

  // stick it into the matrix
  std::vector<unsigned int> di(dof_indices);
  dof_map.constrain_element_matrix(ke, di, false);

  Real scaling_factor = _sys.getVariable(_tid, ivar).scalingFactor();
  if (scaling_factor != 1.0)
  {
    _tmp_Ke = ke;
    _tmp_Ke *= scaling_factor;
    jacobian.add_matrix(_tmp_Ke, di);
  }
  else
    jacobian.add_matrix(ke, di);
}

void
Assembly::addJacobianNeighbor(SparseMatrix<Number> & jacobian, unsigned int ivar, unsigned int jvar, const DofMap & dof_map, std::vector<unsigned int> & dof_indices, std::vector<unsigned int> & neighbor_dof_indices)
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
    _tmp_Ke = kee;
    _tmp_Ke *= scaling_factor;
    jacobian.add_matrix(_tmp_Ke, di);

    _tmp_Ke = ken;
    _tmp_Ke *= scaling_factor;
    jacobian.add_matrix(_tmp_Ke, di, dn);

    _tmp_Ke = kne;
    _tmp_Ke *= scaling_factor;
    jacobian.add_matrix(_tmp_Ke, dn, di);

    _tmp_Ke = knn;
    _tmp_Ke *= scaling_factor;
    jacobian.add_matrix(_tmp_Ke, dn);
  }
  else
  {
    jacobian.add_matrix(kee, di);
    jacobian.add_matrix(ken, di, dn);
    jacobian.add_matrix(kne, dn, di);
    jacobian.add_matrix(knn, dn);
  }
}

void
Assembly::addJacobianScalar(SparseMatrix<Number> & jacobian)
{
  for (unsigned int vi = 0; vi < _sys.nScalarVariables(); ++vi)
  {
    MooseVariableScalar & ivar = _sys.getScalarVariable(_tid, vi);
    for (unsigned int vj = 0; vj < _sys.nScalarVariables(); ++vj)
    {
      MooseVariableScalar & jvar = _sys.getScalarVariable(_tid, vj);
      addJacobianBlock(jacobian, _scalar_Kee[vi][vj], ivar.dofIndices(), jvar.dofIndices(), ivar.scalingFactor());
    }

    if (_scalar_has_off_diag_contributions)
    {
      for (unsigned int vj = 0; vj < _sys.nVariables(); ++vj)
      {
        MooseVariable & jvar = _sys.getVariable(_tid, vj);

        addJacobianBlock(jacobian, _scalar_Ken[vi][vj], ivar.dofIndices(), jvar.dofIndices(), ivar.scalingFactor());
        addJacobianBlock(jacobian, _scalar_Kne[vj][vi], jvar.dofIndices(), ivar.dofIndices(), jvar.scalingFactor());
      }
    }
  }
}

