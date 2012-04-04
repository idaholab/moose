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
#include "Assembly.h"

// libMesh
#include "numeric_vector.h"
#include "dof_map.h"

MooseVariable::MooseVariable(unsigned int var_num, unsigned int mvn, const FEType & fe_type, SystemBase & sys, Assembly & assembly, Moose::VarKindType var_kind) :
    _var_num(var_num),
    _fe_type(fe_type),
    _moose_var_num(mvn),
    _var_kind(var_kind),
    _subproblem(sys.subproblem()),
    _sys(sys),
    _variable(sys.system().variable(_var_num)),
    _dof_map(sys.dofMap()),
    _assembly(assembly),
    _qrule(_assembly.qRule()),
    _qrule_face(_assembly.qRuleFace()),
    _elem(_assembly.elem()),
    _current_side(_assembly.side()),
    _neighbor(_assembly.neighbor()),

    _is_nl(dynamic_cast<NonlinearSystem *>(&sys) != NULL),

    _need_u_old(false),
    _need_u_older(false),

    _need_grad_old(false),
    _need_grad_older(false),

    _need_second(false),
    _need_second_old(false),
    _need_second_older(false),


    _need_u_old_neighbor(false),
    _need_u_older_neighbor(false),

    _need_grad_old_neighbor(false),
    _need_grad_older_neighbor(false),

    _need_second_neighbor(false),
    _need_second_old_neighbor(false),
    _need_second_older_neighbor(false),

    _phi(_assembly.fePhi(_fe_type)),
    _grad_phi(_assembly.feGradPhi(_fe_type)),

    _phi_face(_assembly.fePhiFace(_fe_type)),
    _grad_phi_face(_assembly.feGradPhiFace(_fe_type)),

    _phi_face_neighbor(_assembly.fePhiFaceNeighbor(_fe_type)),
    _grad_phi_face_neighbor(_assembly.feGradPhiFaceNeighbor(_fe_type)),

    _normals(_assembly.normals()),

    _node(_assembly.node()),
    _node_neighbor(_assembly.nodeNeighbor()),
    _scaling_factor(1.0)
{
  // Need to do this to make the assembly aware of what shape functions we're going to use
  _assembly.getFE(_fe_type);
  _assembly.getFEFace(_fe_type);
  _assembly.getFEFaceNeighbor(_fe_type);
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
  _nodal_u_dot.release();
  _nodal_du_dot_du.release();

  _nodal_u_neighbor.release();
  _nodal_u_old_neighbor.release();
  _nodal_u_older_neighbor.release();

  _increment.release();

  _u_neighbor.release();
  _u_old_neighbor.release();
  _u_older_neighbor.release();

  _grad_u_neighbor.release();
  _grad_u_old_neighbor.release();
  _grad_u_older_neighbor.release();

  _second_u_neighbor.release();
  _second_u_old_neighbor.release();
  _second_u_older_neighbor.release();
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
MooseVariable::activeOnSubdomain(subdomain_id_type subdomain) const
{
  return _sys.system().variable(_var_num).active_on_subdomain(subdomain);
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
MooseVariable::reinitNodes(const std::vector<unsigned int> & nodes)
{
  _dof_indices.resize(nodes.size());
  for (unsigned int i = 0; i < nodes.size(); i++)
  {
    Node & nd = _subproblem.mesh().node(nodes[i]);
    if (nd.n_dofs(_sys.number(), _var_num) > 0)
    {
      unsigned int dof = nd.dof_number(_sys.number(), _var_num, 0);
      _dof_indices[i] = dof;
    }
  }
  _is_defined = true;
}

void
MooseVariable::insert(NumericVector<Number> & residual)
{
  if (_has_nodal_value)
    residual.set(_nodal_dof_index, _nodal_u[0]);
}

const std::vector<std::vector<Real> > &
MooseVariable::phi()
{
  return _phi;
}

const std::vector<std::vector<RealGradient> > &
MooseVariable::gradPhi()
{
  return _grad_phi;
}

const std::vector<std::vector<RealTensor> > &
MooseVariable::secondPhi()
{
  _second_phi = &_assembly.feSecondPhi(_fe_type);
  return *_second_phi;
}

const std::vector<std::vector<Real> > &
MooseVariable::phiFace()
{
  return _phi_face;
}

const std::vector<std::vector<RealGradient> > &
MooseVariable::gradPhiFace()
{
  return _grad_phi_face;
}

const std::vector<std::vector<RealTensor> > &
MooseVariable::secondPhiFace()
{
  _second_phi_face = &_assembly.feSecondPhiFace(_fe_type);
  return *_second_phi_face;
}

const std::vector<std::vector<Real> > &
MooseVariable::phiFaceNeighbor()
{
  return _phi_face_neighbor;
}

const std::vector<std::vector<RealGradient> > &
MooseVariable::gradPhiFaceNeighbor()
{
  return _grad_phi_face_neighbor;
}

const std::vector<std::vector<RealTensor> > &
MooseVariable::secondPhiFaceNeighbor()
{
  _second_phi_face_neighbor = &_assembly.feSecondPhiFaceNeighbor(_fe_type);
  return *_second_phi_face_neighbor;
}

void
MooseVariable::computeElemValues()
{

  bool is_transient = _subproblem.isTransient();
  unsigned int nqp = _qrule->n_points();

  _u.resize(nqp);
  _grad_u.resize(nqp);

  if (_need_second)
    _second_u.resize(nqp);

  if (is_transient)
  {
    if (_is_nl)
    {
      _u_dot.resize(nqp);
      _du_dot_du.resize(nqp);
    }

    if(_need_u_old)
      _u_old.resize(nqp);

    if(_need_u_older)
      _u_older.resize(nqp);

    if(_need_grad_old)
      _grad_u_old.resize(nqp);

    if(_need_grad_older)
      _grad_u_older.resize(nqp);

    if(_need_second_old)
      _second_u_old.resize(nqp);

    if(_need_second_older)
      _second_u_older.resize(nqp);
  }

  for (unsigned int i = 0; i < nqp; ++i)
  {
    _u[i] = 0;
    _grad_u[i] = 0;

    if (_need_second)
      _second_u[i] = 0;

    if (is_transient)
    {
      if (_is_nl)
      {
        _u_dot[i] = 0;
        _du_dot_du[i] = 0;
      }

      if(_need_u_old)
        _u_old[i] = 0;

      if(_need_u_older)
        _u_older[i] = 0;

      if(_need_grad_old)
        _grad_u_old[i] = 0;

      if(_need_grad_older)
        _grad_u_older[i] = 0;

      if(_need_second_old)
        _second_u_old[i] = 0;

      if(_need_second_older)
        _second_u_older[i] = 0;
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
  const RealGradient * dphi_qp = NULL;
  const RealTensor * d2phi_local;

  RealGradient * grad_u_qp = NULL;

  RealGradient * grad_u_old_qp = NULL;
  RealGradient * grad_u_older_qp = NULL;

  RealTensor * second_u_qp = NULL;

  RealTensor * second_u_old_qp = NULL;
  RealTensor * second_u_older_qp = NULL;

  for (unsigned int i=0; i < num_dofs; i++)
  {
    idx = _dof_indices[i];
    soln_local = current_solution(idx);

    if (is_transient)
    {
      if(_need_u_old || _need_grad_old || _need_second_old)
        soln_old_local = solution_old(idx);

      if(_need_u_older || _need_grad_older || _need_second_older)
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
      dphi_qp = &_grad_phi[i][qp];

      grad_u_qp = &_grad_u[qp];

      if(is_transient)
      {
        if(_need_grad_old)
          grad_u_old_qp = &_grad_u_old[qp];

        if(_need_grad_older)
          grad_u_older_qp = &_grad_u_older[qp];
      }

      if (_need_second || _need_second_old || _need_second_older)
      {
        d2phi_local = &(*_second_phi)[i][qp];

        if(_need_second)
          second_u_qp = &_second_u[qp];

        if(is_transient)
        {
          if(_need_second_old)
            second_u_old_qp = &_second_u_old[qp];

          if(_need_second_older)
            second_u_older_qp = &_second_u_older[qp];
        }
      }

      _u[qp] += phi_local * soln_local;

      grad_u_qp->add_scaled(*dphi_qp, soln_local);

      if (_need_second)
        second_u_qp->add_scaled(*d2phi_local, soln_local);

      if (is_transient)
      {
        if (_is_nl)
        {
          _u_dot[qp]        += phi_local * u_dot_local;
          _du_dot_du[qp]    += phi_local * du_dot_du_local;
        }

        if(_need_u_old)
          _u_old[qp]        += phi_local * soln_old_local;

        if(_need_u_older)
          _u_older[qp]      += phi_local * soln_older_local;

        if(_need_grad_old)
          grad_u_old_qp->add_scaled(*dphi_qp, soln_old_local);

        if(_need_grad_older)
          grad_u_older_qp->add_scaled(*dphi_qp, soln_older_local);

        if(_need_second_old)
          second_u_old_qp->add_scaled(*d2phi_local, soln_old_local);

        if(_need_second_older)
          second_u_older_qp->add_scaled(*d2phi_local, soln_older_local);
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

  if (_need_second)
    _second_u.resize(nqp);

  if (is_transient)
  {
    if (_is_nl)
    {
      _u_dot.resize(nqp);
      _du_dot_du.resize(nqp);
    }

    if(_need_u_old)
      _u_old.resize(nqp);

    if(_need_u_older)
      _u_older.resize(nqp);

    if(_need_grad_old)
      _grad_u_old.resize(nqp);

    if(_need_grad_older)
      _grad_u_older.resize(nqp);

    if(_need_second_old)
      _second_u_old.resize(nqp);

    if(_need_second_older)
      _second_u_older.resize(nqp);
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

      if(_need_u_old)
        _u_old[i] = 0;

      if(_need_u_older)
        _u_older[i] = 0;

      if(_need_grad_old)
        _grad_u_old[i] = 0;

      if(_need_grad_older)
        _grad_u_older[i] = 0;

      if (_need_second)
        _second_u[i] = 0;

      if(_need_second_old)
        _second_u_old[i] = 0;

      if(_need_second_older)
        _second_u_older[i] = 0;
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
      if(_need_u_old || _need_grad_old || _need_second_old)
        soln_old_local = solution_old(idx);

      if(_need_u_older || _need_grad_older || _need_second_older)
        soln_older_local = solution_older(idx);
    }

    for (unsigned int qp=0; qp < nqp; ++qp)
    {
      phi_local = _phi_face[i][qp];
      dphi_local = _grad_phi_face[i][qp];

      if (_need_second || _need_second_old || _need_second_older)
        d2phi_local = (*_second_phi_face)[i][qp];

      _u[qp]      += phi_local * soln_local;
      _grad_u[qp] += dphi_local * soln_local;

      if (_need_second)
        _second_u[qp] += d2phi_local * soln_local;

      if (is_transient)
      {
        if (_is_nl)
        {
          _u_dot[qp]        += phi_local * u_dot(idx);
          _du_dot_du[qp]    += phi_local * du_dot_du(idx);
        }

        if (_need_u_old)
          _u_old[qp]        += phi_local * soln_old_local;

        if (_need_u_older)
          _u_older[qp]      += phi_local * soln_older_local;

        if(_need_grad_old)
          _grad_u_old[qp]   += dphi_local * soln_old_local;

        if(_need_grad_older)
          _grad_u_older[qp] += dphi_local * soln_older_local;

        if (_need_second_old)
          _second_u_old[qp] += d2phi_local * soln_old_local;

        if (_need_second_older)
          _second_u_older[qp] += d2phi_local * soln_older_local;
      }
    }
  }
}

void
MooseVariable::computeNeighborValuesFace()
{
  bool is_transient = _subproblem.isTransient();
  unsigned int nqp = _qrule_face->n_points();

  _u_neighbor.resize(nqp);
  _grad_u_neighbor.resize(nqp);

  if (_need_second_neighbor)
    _second_u_neighbor.resize(nqp);

  if (is_transient)
  {
    if (_need_u_old_neighbor)
      _u_old_neighbor.resize(nqp);

    if (_need_u_older_neighbor)
      _u_older_neighbor.resize(nqp);

    if (_need_grad_old_neighbor)
      _grad_u_old_neighbor.resize(nqp);

    if (_need_grad_older_neighbor)
      _grad_u_older_neighbor.resize(nqp);

    if (_need_second_old_neighbor)
      _second_u_old_neighbor.resize(nqp);

    if (_need_second_older_neighbor)
      _second_u_older_neighbor.resize(nqp);
  }

  for (unsigned int i = 0; i < nqp; ++i)
  {
    _u_neighbor[i] = 0;
    _grad_u_neighbor[i] = 0;

    if(_need_second_neighbor)
      _second_u_neighbor[i] = 0;

    if (_subproblem.isTransient())
    {
      if (_need_u_old_neighbor)
        _u_old_neighbor[i] = 0;

      if (_need_u_older_neighbor)
        _u_older_neighbor[i] = 0;

      if (_need_grad_old_neighbor)
        _grad_u_old_neighbor[i] = 0;

      if (_need_grad_older_neighbor)
        _grad_u_older_neighbor[i] = 0;

      if (_need_second_old_neighbor)
        _second_u_old_neighbor[i] = 0;

      if (_need_second_older_neighbor)
        _second_u_older_neighbor[i] = 0;
    }
  }

  unsigned int num_dofs = _dof_indices_neighbor.size();

  const NumericVector<Real> & current_solution = *_sys.currentSolution();
  const NumericVector<Real> & solution_old     = _sys.solutionOld();
  const NumericVector<Real> & solution_older   = _sys.solutionOlder();

  int idx;
  Real soln_local;
  Real soln_old_local;
  Real soln_older_local;

  Real phi_local;
  RealGradient dphi_local;
  RealTensor d2phi_local;

  for (unsigned int i=0; i < num_dofs; ++i)
  {
    idx = _dof_indices_neighbor[i];
    soln_local = current_solution(idx);

    if (is_transient)
    {
      if(_need_u_old_neighbor || _need_grad_old_neighbor || _need_second_old_neighbor)
        soln_old_local = solution_old(idx);

      if(_need_u_older_neighbor || _need_grad_older_neighbor || _need_second_older_neighbor)
        soln_older_local = solution_older(idx);
    }

    for (unsigned int qp=0; qp < nqp; ++qp)
    {
      phi_local = _phi_face_neighbor[i][qp];
      dphi_local = _grad_phi_face_neighbor[i][qp];

      if (_need_second_neighbor || _need_second_old_neighbor || _need_second_older_neighbor)
        d2phi_local = (*_second_phi_face_neighbor)[i][qp];

      _u_neighbor[qp]      += phi_local * soln_local;
      _grad_u_neighbor[qp] += dphi_local * soln_local;

      if (_need_second_neighbor)
        _second_u_neighbor[qp] += d2phi_local * soln_local;

      if (is_transient)
      {
        if (_need_u_old_neighbor)
          _u_old_neighbor[qp]        += phi_local * soln_old_local;

        if (_need_u_older_neighbor)
          _u_older_neighbor[qp]      += phi_local * soln_older_local;

        if (_need_grad_old_neighbor)
          _grad_u_old_neighbor[qp]   += dphi_local * soln_old_local;

        if (_need_grad_older_neighbor)
          _grad_u_older_neighbor[qp]   += dphi_local * soln_older_local;

        if (_need_second_old_neighbor)
          _second_u_old_neighbor[qp] += d2phi_local * soln_old_local;

        if (_need_second_older_neighbor)
          _second_u_older_neighbor[qp] += d2phi_local * soln_older_local;
      }
    }
  }
}

void
MooseVariable::computeNeighborValues()
{
  bool is_transient = _subproblem.isTransient();
  unsigned int nqp = _qrule->n_points();

  _u_neighbor.resize(nqp);
  _grad_u_neighbor.resize(nqp);

  if (_need_second_neighbor)
    _second_u_neighbor.resize(nqp);

  if (is_transient)
  {
    if(_need_u_old_neighbor)
      _u_old_neighbor.resize(nqp);

    if(_need_u_older_neighbor)
      _u_older_neighbor.resize(nqp);

    if(_need_grad_old_neighbor)
      _grad_u_old_neighbor.resize(nqp);

    if(_need_grad_older_neighbor)
      _grad_u_older_neighbor.resize(nqp);

    if(_need_second_old_neighbor)
      _second_u_old_neighbor.resize(nqp);

    if(_need_second_older_neighbor)
      _second_u_older_neighbor.resize(nqp);
  }

  for (unsigned int i = 0; i < nqp; ++i)
  {
    _u_neighbor[i] = 0;
    _grad_u_neighbor[i] = 0;

    if (_subproblem.isTransient())
    {
      if(_need_u_old_neighbor)
        _u_old_neighbor[i] = 0;

      if(_need_u_older_neighbor)
        _u_older_neighbor[i] = 0;

      if(_need_grad_old_neighbor)
        _grad_u_old_neighbor[i] = 0;

      if(_need_grad_older_neighbor)
        _grad_u_older_neighbor[i] = 0;

      if(_need_second_old_neighbor)
        _second_u_old_neighbor[i] = 0;

      if(_need_second_older_neighbor)
        _second_u_older_neighbor[i] = 0;
    }
  }

  unsigned int num_dofs = _dof_indices_neighbor.size();

  const NumericVector<Real> & current_solution = *_sys.currentSolution();
  const NumericVector<Real> & solution_old     = _sys.solutionOld();
  const NumericVector<Real> & solution_older   = _sys.solutionOlder();

  int idx;
  Real soln_local;
  Real soln_old_local;
  Real soln_older_local;

  Real phi_local;
  RealGradient dphi_local;
  RealTensor d2phi_local;

  for (unsigned int i=0; i < num_dofs; ++i)
  {
    idx = _dof_indices_neighbor[i];
    soln_local = current_solution(idx);

    if (is_transient)
    {
      if(_need_u_old_neighbor)
        soln_old_local = solution_old(idx);

      if(_need_u_older_neighbor)
        soln_older_local = solution_older(idx);
    }

    for (unsigned int qp=0; qp < nqp; ++qp)
    {
      phi_local = _phi_face_neighbor[i][qp];
      dphi_local = _grad_phi_face_neighbor[i][qp];

      if (_need_second_neighbor || _need_second_old_neighbor || _need_second_older_neighbor)
        d2phi_local = (*_second_phi_face_neighbor)[i][qp];

      _u_neighbor[qp]      += phi_local * soln_local;
      _grad_u_neighbor[qp] += dphi_local * soln_local;

      if (_need_second_neighbor)
        _second_u_neighbor[qp] += d2phi_local * soln_local;

      if (is_transient)
      {
        if (_need_u_old_neighbor)
          _u_old_neighbor[qp]        += phi_local * soln_old_local;

        if (_need_u_older_neighbor)
          _u_older_neighbor[qp]      += phi_local * soln_older_local;

        if (_need_grad_old_neighbor)
          _grad_u_old_neighbor[qp]   += dphi_local * soln_old_local;

        if (_need_grad_older_neighbor)
          _grad_u_older_neighbor[qp]   += dphi_local * soln_older_local;

        if (_need_second_old_neighbor)
          _second_u_old_neighbor[qp] += d2phi_local * soln_old_local;

        if (_need_second_older_neighbor)
          _second_u_older_neighbor[qp] += d2phi_local * soln_older_local;
      }
    }
  }
}

void
MooseVariable::computeNodalValues()
{
  if (_is_defined)
  {
    unsigned int n = _dof_indices.size();

    _nodal_u.resize(n);
    if (_subproblem.isTransient())
    {
      _nodal_u_old.resize(n);
      _nodal_u_older.resize(n);
      if (_is_nl)
      {
        _nodal_u_dot.resize(n);
        _nodal_du_dot_du.resize(n);
      }
    }

    for (unsigned int i = 0; i < n; i++)
    {
      _nodal_u[i] = (*_sys.currentSolution())(_dof_indices[i]);

      if (_subproblem.isTransient())
      {
        _nodal_u_old[i] = _sys.solutionOld()(_dof_indices[i]);
        _nodal_u_older[i] = _sys.solutionOlder()(_dof_indices[i]);

        if (_is_nl)
        {
          _nodal_u_dot[i] = _sys.solutionUDot()(_dof_indices[i]);
          _nodal_du_dot_du[i] = _sys.solutionDuDotDu()(_dof_indices[i]);
        }
      }
    }
  }
}

void
MooseVariable::computeNodalNeighborValues()
{
  if (_is_defined_neighbor)
  {
    unsigned int n = _dof_indices_neighbor.size();

    _nodal_u_neighbor.resize(n);
    if (_subproblem.isTransient())
    {
      _nodal_u_old_neighbor.resize(n);
      _nodal_u_older_neighbor.resize(n);
    }

    for (unsigned int i = 0; i < n; i++)
    {
      _nodal_u_neighbor[i] = (*_sys.currentSolution())(_dof_indices_neighbor[i]);
      if (_subproblem.isTransient())
      {
        _nodal_u_old_neighbor[i] = _sys.solutionOld()(_dof_indices_neighbor[i]);
        _nodal_u_older_neighbor[i] = _sys.solutionOlder()(_dof_indices_neighbor[i]);
      }
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
