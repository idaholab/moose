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

#include "MooseVariable.h"
#include "SubProblem.h"
#include "SystemBase.h"
#include "Assembly.h"
#include "NonlinearSystem.h"

// libMesh
#include "numeric_vector.h"
#include "dof_map.h"

#if 0
// VariableData /////

VariableData::VariableData(THREAD_ID tid, const FEType & fe_type, SystemBase & sys) :
    _subproblem(sys.problem()),
    _sys(sys),
    _fe(_subproblem.getFE(tid, fe_type)),
    _qrule(_subproblem.qRule(tid)),
    _phi(_fe->get_phi()),
    _grad_phi(_fe->get_dphi())
{

}

void
VariableData::computeValues()
{
  unsigned int nqp = _qrule->n_points();
  _u.resize(nqp);
  _grad_u.resize(nqp);
  if (_subproblem.isTransient())
  {
    _u_dot.resize(nqp);
    _du_dot_du.resize(nqp);

    _u_old.resize(nqp);
    _u_older.resize(nqp);

    _grad_u_old.resize(nqp);
    _grad_u_older.resize(nqp);
  }

  for (unsigned int i = 0; i < nqp; ++i)
  {
    _u[i] = 0;
    _grad_u[i] = 0;

    if (_subproblem.isTransient())
    {
      _u_dot[i] = 0;
      _du_dot_du[i] = 0;

      _u_old[i] = 0;
      _u_older[i] = 0;

      _grad_u_old[i] = 0;
      _grad_u_older[i] = 0;
    }
  }

  unsigned int num_dofs = _dof_indices.size();
  for (unsigned int i = 0; i < num_dofs; i++)
  {
    int idx = _dof_indices[i];
    Real soln_local = (*_sys.currentSolution())(idx);
    Real soln_old_local;
    Real soln_older_local;

    if (_subproblem.transient())
    {
      soln_old_local = _sys.solutionOld()(idx);
      soln_older_local = _sys.solutionOlder()(idx);
    }

    for (unsigned int qp = 0; qp < nqp; qp++)
    {
      Real phi_local = _phi[i][qp];
      RealGradient dphi_local = _grad_phi[i][qp];

      _u[qp]      += phi_local * soln_local;
      _grad_u[qp] += dphi_local * soln_local;

      if (_subproblem.isTransient())
      {
        _u_dot[qp]        += phi_local * _sys.solutionUDot()(idx);
        _du_dot_du[qp]    += phi_local * _sys.solutionDuDotDu()(idx);

        _u_old[qp]        += phi_local * soln_old_local;
        _u_older[qp]      += phi_local * soln_older_local;
        _grad_u_old[qp]   += dphi_local * soln_old_local;
        _grad_u_older[qp] += dphi_local * soln_older_local;
      }
    }
  }
}
#endif

// Variable /////

MooseVariable::MooseVariable(unsigned int var_num, const FEType & fe_type, SystemBase & sys, Assembly & assembly) :
    _var_num(var_num),
    _subproblem(sys.subproblem()),
    _sys(sys),
    _dof_map(sys.dofMap()),
    _assembly(assembly),
    _qrule(_assembly.qRule()),
    _qrule_face(_assembly.qRuleFace()),
    _fe(_assembly.getFE(fe_type)),
    _fe_face(_assembly.getFEFace(fe_type)),
    _fe_face_neighbor(_assembly.getFEFaceNeighbor(fe_type)),
    _elem(_assembly.elem()),
    _current_side(_assembly.side()),
    _neighbor(_assembly.neighbor()),

    _is_nl(dynamic_cast<NonlinearSystem *>(&sys) != NULL),
    _has_second_derivatives(fe_type.family == CLOUGH || fe_type.family == HERMITE),

    _phi(_fe->get_phi()),
    _grad_phi(_fe->get_dphi()),
    _second_phi(_has_second_derivatives ? _fe->get_d2phi() : _dummy_second_phi),

    _phi_face(_fe_face->get_phi()),
    _grad_phi_face(_fe_face->get_dphi()),
    _second_phi_face(_has_second_derivatives ? _fe_face->get_d2phi() : _dummy_second_phi),

    _phi_face_neighbor(_fe_face_neighbor->get_phi()),
    _grad_phi_face_neighbor(_fe_face_neighbor->get_dphi()),
    _second_phi_face_neighbor(_has_second_derivatives ? _fe_face_neighbor->get_d2phi() : _dummy_second_phi),

    _normals(_fe_face->get_normals()),

    _node(_assembly.node()),
    _node_neighbor(_assembly.nodeNeighbor()),
    _scaling_factor(1.0)
{
}

MooseVariable::~MooseVariable()
{
  _u.release();
  _u_old.release();
  _u_older.release();

  _grad_u.release();
  _grad_u_old.release();
  _grad_u_older.release();

  _second_u.release();
  _second_u_old.release();
  _second_u_older.release();

  _u_dot.release();
  _du_dot_du.release();

  _nodal_u.release();
  _nodal_u_old.release();
  _nodal_u_older.release();

  _nodal_u_neighbor.release();
  _nodal_u_old_neighbor.release();
  _nodal_u_older_neighbor.release();

  _increment.release();

  _u_neighbor.release();
  _u_old_neighbor.release();
  _u_older_neighbor.release();

  _grad_u_neighbor.release();
  _grad_u_old_neighbor.release();

  _second_u_neighbor.release();
}

const std::string &
MooseVariable::name()
{
  return _sys.system().variable(_var_num).name();
}

const std::set<subdomain_id_type> &
MooseVariable::activeSubdomains()
{
  return _sys.system().variable(_var_num).active_subdomains();
}

bool
MooseVariable::isNodal()
{
  // FIXME: improve this fix (currently we use only monomials as elemental variables)
  return feType().family != MONOMIAL;
}

void
MooseVariable::prepare()
{
  _dof_map.dof_indices (_elem, _dof_indices, _var_num);
}

void
MooseVariable::prepareNeighbor()
{
  _dof_map.dof_indices (_neighbor, _dof_indices_neighbor, _var_num);
}

void
MooseVariable::prepare_aux()
{
  _has_nodal_value = false;
}

void
MooseVariable::reinit_node()
{
  if (_node->n_dofs(_sys.number(), _var_num) > 0)
  {
    _nodal_dof_index = _node->dof_number(_sys.number(), _var_num, 0);
    _dof_indices.resize(1);
    _dof_indices[0] = _nodal_dof_index;
    _is_defined = true;
  }
  else
    _is_defined = false;
}

void
MooseVariable::reinit_nodeNeighbor()
{
  if (_node_neighbor->n_dofs(_sys.number(), _var_num) > 0)
  {
    _nodal_dof_index_neighbor = _node_neighbor->dof_number(_sys.number(), _var_num, 0);
    _dof_indices_neighbor.resize(1);
    _dof_indices_neighbor[0] = _nodal_dof_index_neighbor;
    _is_defined_neighbor = true;
  }
  else
    _is_defined_neighbor = false;
}


void
MooseVariable::reinit_aux()
{
  _dof_map.dof_indices (_elem, _dof_indices, _var_num);
  if (_elem->n_dofs(_sys.number(), _var_num) > 0)
  {
    _nodal_dof_index = _elem->dof_number(_sys.number(), _var_num, 0);
    _is_defined = true;
  }
  else
    _is_defined = false;
}

void
MooseVariable::insert(NumericVector<Number> & residual)
{
  if (_has_nodal_value)
    residual.set(_nodal_dof_index, _nodal_u[0]);
}

void
MooseVariable::computeElemValues()
{

  bool is_transient = _subproblem.isTransient();
  unsigned int nqp = _qrule->n_points();

  _u.resize(nqp);
  _grad_u.resize(nqp);
  if (_has_second_derivatives)
    _second_u.resize(nqp);
  if (is_transient)
  {
    if (_is_nl)
    {
      _u_dot.resize(nqp);
      _du_dot_du.resize(nqp);
    }

    _u_old.resize(nqp);
    _u_older.resize(nqp);
    _grad_u_old.resize(nqp);
    _grad_u_older.resize(nqp);
    if (_has_second_derivatives)
    {
      _second_u_old.resize(nqp);
      _second_u_older.resize(nqp);
    }
  }

  for (unsigned int i = 0; i < nqp; ++i)
  {
    _u[i] = 0;
    _grad_u[i] = 0;
    if (_has_second_derivatives)
      _second_u[i] = 0;

    if (is_transient)
    {
      if (_is_nl)
      {
        _u_dot[i] = 0;
        _du_dot_du[i] = 0;
      }

      _u_old[i] = 0;
      _u_older[i] = 0;
      _grad_u_old[i] = 0;
      _grad_u_older[i] = 0;
      if (_has_second_derivatives)
      {
        _second_u_old[i] = 0;
        _second_u_older[i] = 0;
      }
    }
  }

  unsigned int num_dofs = _dof_indices.size();

  const NumericVector<Real> & current_solution = *_sys.currentSolution();
  const NumericVector<Real> & solution_old     = _sys.solutionOld();
  const NumericVector<Real> & solution_older   = _sys.solutionOlder();
  const NumericVector<Real> & u_dot            = _sys.solutionUDot();
  const NumericVector<Real> & du_dot_du        = _sys.solutionDuDotDu();

  int idx = 0;
  Real soln_local = 0;
  Real soln_old_local = 0;
  Real soln_older_local = 0;
  Real u_dot_local = 0;
  Real du_dot_du_local = 0;

  Real phi_local = 0;
  const Real * dphi_qp = NULL;
  //RealGradient dphi_local;
  RealTensor d2phi_local;

  Real * grad_u_qp = NULL;
  Real * grad_u_old_qp = NULL;
  Real * grad_u_older_qp = NULL;

  for (unsigned int i=0; i < num_dofs; i++)
  {
    idx = _dof_indices[i];
    soln_local = current_solution(idx);

    if (is_transient)
    {
      soln_old_local = solution_old(idx);
      soln_older_local = solution_older(idx);

      if (_is_nl)
      {
        u_dot_local        = u_dot(idx);
        du_dot_du_local    = du_dot_du(idx);
      }
    }

    for (unsigned int qp=0; qp < nqp; qp++)
    {
      phi_local = _phi[i][qp];
      dphi_qp = &_grad_phi[i][qp](0);

      grad_u_qp = &_grad_u[qp](0);

      if(is_transient)
      {
        grad_u_old_qp = &_grad_u_old[qp](0);
        grad_u_older_qp = &_grad_u_older[qp](0);
      }

      if (_has_second_derivatives)
        d2phi_local = _second_phi[i][qp];

      _u[qp] += phi_local * soln_local;

      for(unsigned int j=0; j < LIBMESH_DIM; ++j)
        grad_u_qp[j] += dphi_qp[j] * soln_local;

      if (_has_second_derivatives)
        _second_u[qp] += d2phi_local * soln_local;

      if (is_transient)
      {
        if (_is_nl)
        {
          _u_dot[qp]        += phi_local * u_dot_local;
          _du_dot_du[qp]    += phi_local * du_dot_du_local;
        }

        _u_old[qp]        += phi_local * soln_old_local;
        _u_older[qp]      += phi_local * soln_older_local;
        //_grad_u_old[qp]   += dphi_local * soln_old_local;
        for(unsigned int j=0; j < LIBMESH_DIM; ++j)
          grad_u_old_qp[j] += dphi_qp[j] * soln_old_local;

        //_grad_u_older[qp] += dphi_local * soln_older_local;
        for(unsigned int j=0; j < LIBMESH_DIM; ++j)
          grad_u_older_qp[j] += dphi_qp[j] * soln_older_local;
        if (_has_second_derivatives)
        {
          _second_u_old[qp] += d2phi_local * soln_old_local;
          _second_u_older[qp] += d2phi_local * soln_older_local;
        }
      }
    }
  }
}

void
MooseVariable::computeElemValuesFace()
{
  bool is_transient = _subproblem.isTransient();
  unsigned int nqp = _qrule_face->n_points();
  _u.resize(nqp);
  _grad_u.resize(nqp);
  if (_has_second_derivatives)
    _second_u.resize(nqp);
  if (is_transient)
  {
    if (_is_nl)
    {
      _u_dot.resize(nqp);
      _du_dot_du.resize(nqp);
    }

    _u_old.resize(nqp);
    _u_older.resize(nqp);
    _grad_u_old.resize(nqp);
    _grad_u_older.resize(nqp);
    if (_has_second_derivatives)
    {
      _second_u_old.resize(nqp);
      _second_u_older.resize(nqp);
    }
  }

  for (unsigned int i = 0; i < nqp; ++i)
  {
    _u[i] = 0;
    _grad_u[i] = 0;

    if (_subproblem.isTransient())
    {
      if (_is_nl)
      {
        _u_dot[i] = 0;
        _du_dot_du[i] = 0;
      }

      _u_old[i] = 0;
      _u_older[i] = 0;
      _grad_u_old[i] = 0;
      _grad_u_older[i] = 0;
      if (_has_second_derivatives)
      {
        _second_u_old[i] = 0;
        _second_u_older[i] = 0;
      }
    }
  }

  unsigned int num_dofs = _dof_indices.size();

  const NumericVector<Real> & current_solution = *_sys.currentSolution();
  const NumericVector<Real> & solution_old     = _sys.solutionOld();
  const NumericVector<Real> & solution_older   = _sys.solutionOlder();
  const NumericVector<Real> & u_dot            = _sys.solutionUDot();
  const NumericVector<Real> & du_dot_du        = _sys.solutionDuDotDu();

  int idx;
  Real soln_local;
  Real soln_old_local;
  Real soln_older_local;
//  Real u_dot_local;
//  Real du_dot_du_local;

  Real phi_local;
  RealGradient dphi_local;
  RealTensor d2phi_local;

  for (unsigned int i=0; i < num_dofs; ++i)
  {
    idx = _dof_indices[i];
    soln_local = current_solution(idx);

    if (is_transient)
    {
      soln_old_local = solution_old(idx);
      soln_older_local = solution_older(idx);
    }

    for (unsigned int qp=0; qp < nqp; ++qp)
    {
      phi_local = _phi_face[i][qp];
      dphi_local = _grad_phi_face[i][qp];

      if (_has_second_derivatives)
        d2phi_local = _second_phi_face[i][qp];

      _u[qp]      += phi_local * soln_local;
      _grad_u[qp] += dphi_local * soln_local;
      if (_has_second_derivatives)
        _second_u[qp] += d2phi_local * soln_local;

      if (is_transient)
      {
        if (_is_nl)
        {
          _u_dot[qp]        += phi_local * u_dot(idx);
          _du_dot_du[qp]    += phi_local * du_dot_du(idx);
        }

        _u_old[qp]        += phi_local * soln_old_local;
        _u_older[qp]      += phi_local * soln_older_local;
        _grad_u_old[qp]   += dphi_local * soln_old_local;
        _grad_u_older[qp] += dphi_local * soln_older_local;
        if (_has_second_derivatives)
        {
          _second_u_old[qp] += d2phi_local * soln_old_local;
          _second_u_older[qp] += d2phi_local * soln_older_local;
        }
      }
    }
  }
}

void
MooseVariable::computeNeighborValuesFace()
{
  bool is_transient = _subproblem.isTransient();
  unsigned int nqp = _qrule_face->n_points();
//  std::cerr << "nqp = " << nqp << std::endl;
  _u_neighbor.resize(nqp);
  _grad_u_neighbor.resize(nqp);
  if (_has_second_derivatives)
    _second_u_neighbor.resize(nqp);
  if (is_transient)
  {
    _u_old_neighbor.resize(nqp);
    _u_older_neighbor.resize(nqp);
    _grad_u_old_neighbor.resize(nqp);
  }

  for (unsigned int i = 0; i < nqp; ++i)
  {
    _u_neighbor[i] = 0;
    _grad_u_neighbor[i] = 0;

    if (_subproblem.isTransient())
    {
      _u_old_neighbor[i] = 0;
      _u_older_neighbor[i] = 0;
      _grad_u_old_neighbor[i] = 0;
    }
  }

  unsigned int num_dofs = _dof_indices_neighbor.size();
//  std::cerr << "num_dofs = " << num_dofs << std::endl;

  const NumericVector<Real> & current_solution = *_sys.currentSolution();
  const NumericVector<Real> & solution_old     = _sys.solutionOld();
  const NumericVector<Real> & solution_older   = _sys.solutionOlder();
//  const NumericVector<Real> & u_dot            = _sys.solutionUDot();
//  const NumericVector<Real> & du_dot_du        = _sys.solutionDuDotDu();

  int idx;
  Real soln_local;
  Real soln_old_local;
  Real soln_older_local;
//  Real u_dot_local;
//  Real du_dot_du_local;

  Real phi_local;
  RealGradient dphi_local;
  RealTensor d2phi_local;

  for (unsigned int i=0; i < num_dofs; ++i)
  {
    idx = _dof_indices_neighbor[i];
    soln_local = current_solution(idx);

//    std::cerr << idx << " " << ": ";

    if (is_transient)
    {
      soln_old_local = solution_old(idx);
      soln_older_local = solution_older(idx);
    }

    for (unsigned int qp=0; qp < nqp; ++qp)
    {
      phi_local = _phi_face_neighbor[i][qp];
      dphi_local = _grad_phi_face_neighbor[i][qp];

      if (_has_second_derivatives)
        d2phi_local = _second_phi_face_neighbor[i][qp];

//      std::cerr << phi_local << " * " << soln_local << ", ";
      _u_neighbor[qp]      += phi_local * soln_local;
      _grad_u_neighbor[qp] += dphi_local * soln_local;
      if (_has_second_derivatives)
        _second_u_neighbor[qp] += d2phi_local * soln_local;

      if (is_transient)
      {
        _u_old_neighbor[qp]        += phi_local * soln_old_local;
        _u_older_neighbor[qp]      += phi_local * soln_older_local;
        _grad_u_old_neighbor[qp]   += dphi_local * soln_old_local;
      }
    }
//    std::cerr << " => " << _u_neighbor[0] << std::endl;

//    printf(" => % .25f", _u_neighbor[0]);
//    std::cerr << std::endl;

  }

//  std::cerr << " _u_n = ";
//  for (unsigned int qp=0; qp < nqp; ++qp)
//    printf("% .25f ", _u_neighbor[qp]);
////    std::cerr << _u_neighbor[qp] << " ";
//  std::cerr << std::endl;
}

void
MooseVariable::computeNeighborValues()
{
  bool is_transient = _subproblem.isTransient();
  unsigned int nqp = _qrule->n_points();
//  std::cerr << "nqp = " << nqp << std::endl;
  _u_neighbor.resize(nqp);
  _grad_u_neighbor.resize(nqp);
  if (_has_second_derivatives)
    _second_u_neighbor.resize(nqp);
  if (is_transient)
  {
    _u_old_neighbor.resize(nqp);
    _u_older_neighbor.resize(nqp);
    _grad_u_old_neighbor.resize(nqp);
  }

  for (unsigned int i = 0; i < nqp; ++i)
  {
    _u_neighbor[i] = 0;
    _grad_u_neighbor[i] = 0;

    if (_subproblem.isTransient())
    {
      _u_old_neighbor[i] = 0;
      _u_older_neighbor[i] = 0;
      _grad_u_old_neighbor[i] = 0;
    }
  }

  unsigned int num_dofs = _dof_indices_neighbor.size();
//  std::cerr << "num_dofs = " << num_dofs << std::endl;

  const NumericVector<Real> & current_solution = *_sys.currentSolution();
  const NumericVector<Real> & solution_old     = _sys.solutionOld();
  const NumericVector<Real> & solution_older   = _sys.solutionOlder();
//  const NumericVector<Real> & u_dot            = _sys.solutionUDot();
//  const NumericVector<Real> & du_dot_du        = _sys.solutionDuDotDu();

  int idx;
  Real soln_local;
  Real soln_old_local;
  Real soln_older_local;
//  Real u_dot_local;
//  Real du_dot_du_local;

  Real phi_local;
  RealGradient dphi_local;
  RealTensor d2phi_local;

  for (unsigned int i=0; i < num_dofs; ++i)
  {
    idx = _dof_indices_neighbor[i];
    soln_local = current_solution(idx);

//    std::cerr << idx << " " << ": ";

    if (is_transient)
    {
      soln_old_local = solution_old(idx);
      soln_older_local = solution_older(idx);
    }

    for (unsigned int qp=0; qp < nqp; ++qp)
    {
      phi_local = _phi_face_neighbor[i][qp];
      dphi_local = _grad_phi_face_neighbor[i][qp];

      if (_has_second_derivatives)
        d2phi_local = _second_phi_face_neighbor[i][qp];

//      std::cerr << phi_local << " * " << soln_local << ", ";
      _u_neighbor[qp]      += phi_local * soln_local;
      _grad_u_neighbor[qp] += dphi_local * soln_local;
      if (_has_second_derivatives)
        _second_u_neighbor[qp] += d2phi_local * soln_local;

      if (is_transient)
      {
        _u_old_neighbor[qp]        += phi_local * soln_old_local;
        _u_older_neighbor[qp]      += phi_local * soln_older_local;
        _grad_u_old_neighbor[qp]   += dphi_local * soln_old_local;
      }
    }
//    std::cerr << " => " << _u_neighbor[0] << std::endl;

//    printf(" => % .25f", _u_neighbor[0]);
//    std::cerr << std::endl;

  }

//  std::cerr << " _u_n = ";
//  for (unsigned int qp=0; qp < nqp; ++qp)
//    printf("% .25f ", _u_neighbor[qp]);
////    std::cerr << _u_neighbor[qp] << " ";
//  std::cerr << std::endl;
}

void
MooseVariable::computeNodalValues()
{
  if (_is_defined)
  {
    _nodal_u.resize(1);
    _nodal_u[0] = (*_sys.currentSolution())(_nodal_dof_index);

    if (_subproblem.isTransient())
    {
      _nodal_u_old.resize(1);
      _nodal_u_old[0] = _sys.solutionOld()(_nodal_dof_index);

      _nodal_u_older.resize(1);
      _nodal_u_older[0] = _sys.solutionOlder()(_nodal_dof_index);
    }
  }
}

void
MooseVariable::computeNodalNeighborValues()
{
  if (_is_defined_neighbor)
  {
    _nodal_u_neighbor.resize(1);
    _nodal_u_neighbor[0] = (*_sys.currentSolution())(_nodal_dof_index_neighbor);

    if (_subproblem.isTransient())
    {
      _nodal_u_old_neighbor.resize(1);
      _nodal_u_old_neighbor[0] = _sys.solutionOld()(_nodal_dof_index_neighbor);

      _nodal_u_older_neighbor.resize(1);
      _nodal_u_older_neighbor[0] = _sys.solutionOlder()(_nodal_dof_index_neighbor);
    }
  }
}

void
MooseVariable::setNodalValue(Number value)
{
  _nodal_u.resize(1);
  _nodal_u[0] = value;                  // update variable nodal value
  _has_nodal_value = true;
}

void
MooseVariable::computeDamping(const NumericVector<Number> & increment_vec)
{
  unsigned int nqp = _qrule->n_points();

  _increment.resize(nqp);
  // Compute the increment at each quadrature point
  unsigned int num_dofs = _dof_indices.size();
  for(unsigned int qp=0; qp<nqp; qp++)
  {
    _increment[qp]=0;
    for (unsigned int i=0; i<num_dofs; i++)
      _increment[qp] +=  _phi[i][qp]*increment_vec(_dof_indices[i]);
  }
}

Number
MooseVariable::getNodalValue(const Node & node)
{
  unsigned int dof = node.dof_number(_sys.number(), _var_num, 0);
  return (*_sys.currentSolution())(dof);

}

Number
MooseVariable::getNodalValueOld(const Node & node)
{
  unsigned int dof = node.dof_number(_sys.number(), _var_num, 0);
  return _sys.solutionOld()(dof);

}

Number
MooseVariable::getNodalValueOlder(const Node & node)
{
  unsigned int dof = node.dof_number(_sys.number(), _var_num, 0);
  return _sys.solutionOlder()(dof);
}
