#include "Variable.h"
#include "SubProblem.h"
#include "System.h"

// libMesh
#include "numeric_vector.h"
#include "dof_map.h"

namespace Moose {

Variable::Variable(unsigned int var_num, int dim, const FEType & fe_type, System & sys) :
    _var_num(var_num),
    _dim(dim),
    _fe_type(fe_type),
    _problem(sys.problem()),
    _sys(sys),
    _dof_map(sys.dofMap()),
    _qrule(NULL),
    _fe(FEBase::build(dim, fe_type).release()),
    _elem(NULL),
    _qpoints(_fe->get_xyz()),
    _JxW(_fe->get_JxW()),
    _phi(_fe->get_phi()),
    _grad_phi(_fe->get_dphi()),
    _normals(_fe->get_normals()),
    _node(NULL)
{
}

Variable::~Variable()
{
}

void
Variable::attachQuadratureRule(QBase *qrule)
{
  _fe->attach_quadrature_rule(qrule);
  _qrule = qrule;
}

void
Variable::reinit(const Elem *elem)
{
  _elem = elem;
  _fe->reinit(elem);
  _dof_map.dof_indices (elem, _dof_indices, _var_num);
}

void
Variable::reinit(const Elem *elem, unsigned int side)
{
  _elem = elem;
  _current_side = side;
  _fe->reinit(elem, side);
  _dof_map.dof_indices (elem, _dof_indices, _var_num);
}


void
Variable::reinit(const Node * node)
{
  _node = node;
  _nodal_dof_index = node->dof_number(_sys.number(), _var_num, 0);
}

void
Variable::reinit_aux(const Elem *elem)
{
  reinit(elem);
  _nodal_dof_index = elem->dof_number(_sys.number(), _var_num, 0);
}

void
Variable::sizeResidual()
{
  _Re.resize(_dof_indices.size());
  _Re.zero();
}

void
Variable::sizeJacobianBlock()
{
  _Ke.resize(_dof_indices.size(), _dof_indices.size());
}

void
Variable::add(NumericVector<Number> & residual)
{
  _dof_map.constrain_element_vector(_Re, _dof_indices, false);
  residual.add_vector(_Re, _dof_indices);
}

void
Variable::add(SparseMatrix<Number> & jacobian)
{
  _dof_map.constrain_element_matrix(_Ke, _dof_indices, false);
  jacobian.add_matrix(_Ke, _dof_indices);
}

void
Variable::computeElemValues()
{
  unsigned int nqp = _qrule->n_points();
  _u.resize(nqp);
  _grad_u.resize(nqp);
  if (_problem.transient())
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

    if (_problem.transient())
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
    Real soln_local = _sys.solution()(idx);
    Real soln_old_local;
    Real soln_older_local;

    if (_problem.transient())
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

      if (_problem.transient())
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

void
Variable::computeNodalValues()
{
  _nodal_u.resize(1);
  _nodal_u[0] = _sys.solution()(_nodal_dof_index);
}

} // namespace
