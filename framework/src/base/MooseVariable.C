#include "MooseVariable.h"
#include "SubProblem.h"
#include "SystemBase.h"
#include "AssemblyData.h"

// libMesh
#include "numeric_vector.h"
#include "dof_map.h"

#if 0
// VariableData /////

VariableData::VariableData(THREAD_ID tid, const FEType & fe_type, SystemBase & sys) :
    _problem(sys.problem()),
    _sys(sys),
    _fe(_problem.getFE(tid, fe_type)),
    _qrule(_problem.qRule(tid)),
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
#endif

// Variable /////

MooseVariable::MooseVariable(unsigned int var_num, const FEType & fe_type, SystemBase & sys, AssemblyData & assembly_data) :
    _var_num(var_num),
    _problem(sys.problem()),
    _sys(sys),
    _dof_map(sys.dofMap()),
    _assembly(assembly_data),
    _qrule(_assembly.qRule()),
    _qrule_face(_assembly.qRuleFace()),
    _fe(_assembly.getFE(fe_type)),
    _fe_face(_assembly.getFEFace(fe_type)),
    _elem(_assembly.elem()),
    _current_side(_assembly.side()),

    _phi(_fe->get_phi()),
    _grad_phi(_fe->get_dphi()),
    _second_phi(_fe->get_d2phi()),

    _phi_face(_fe_face->get_phi()),
    _grad_phi_face(_fe_face->get_dphi()),
    _second_phi_face(_fe_face->get_d2phi()),
    _normals(_fe_face->get_normals()),

    _node(_assembly.node())
{
}

MooseVariable::~MooseVariable()
{
}

void
MooseVariable::prepare()
{
  _dof_map.dof_indices (_elem, _dof_indices, _var_num);
}

void
MooseVariable::reinit()
{
  // copy shape functions into test functions (so they can be modified by stabilizers)
  _test = _phi;
  _grad_test = _grad_phi;
  _second_test = _second_phi;

  // FIXME: move to face reinit
  _test_face = _phi_face;
  _grad_test_face = _grad_phi_face;
  _second_test_face = _second_phi_face;

}

void
MooseVariable::reinit_node()
{
  _nodal_dof_index = _node->dof_number(_sys.number(), _var_num, 0);
}

void
MooseVariable::reinit_aux()
{
  reinit();
  _dof_map.dof_indices (_elem, _dof_indices, _var_num);
  _nodal_dof_index = _elem->dof_number(_sys.number(), _var_num, 0);
}

void
MooseVariable::sizeResidual()
{
  _Re.resize(_dof_indices.size());
  _Re.zero();
}

void
MooseVariable::sizeJacobianBlock()
{
  _Ke.resize(_dof_indices.size(), _dof_indices.size());
}

void
MooseVariable::add(NumericVector<Number> & residual)
{
  _dof_map.constrain_element_vector(_Re, _dof_indices, false);
  residual.add_vector(_Re, _dof_indices);
}

void
MooseVariable::add(SparseMatrix<Number> & jacobian)
{
  _dof_map.constrain_element_matrix(_Ke, _dof_indices, false);
  jacobian.add_matrix(_Ke, _dof_indices);
}

void
MooseVariable::computeElemValues()
{
  bool has_second_derivatives = (feType().family == CLOUGH || feType().family == HERMITE);

  unsigned int nqp = _qrule->n_points();
  _u.resize(nqp);
  _grad_u.resize(nqp);
  if (has_second_derivatives)
    _second_u.resize(nqp);
  if (_problem.transient())
  {
    _u_dot.resize(nqp);
    _du_dot_du.resize(nqp);

    _u_old.resize(nqp);
    _u_older.resize(nqp);
    _grad_u_old.resize(nqp);
    _grad_u_older.resize(nqp);
    if (has_second_derivatives)
    {
      _second_u_old.resize(nqp);
      _second_u_older.resize(nqp);
    }
  }

  for (unsigned int i = 0; i < nqp; ++i)
  {
    _u[i] = 0;
    _grad_u[i] = 0;
    if (has_second_derivatives)
      _second_u[i] = 0;

    if (_problem.transient())
    {
      _u_dot[i] = 0;
      _du_dot_du[i] = 0;

      _u_old[i] = 0;
      _u_older[i] = 0;
      _grad_u_old[i] = 0;
      _grad_u_older[i] = 0;
      if (has_second_derivatives)
      {
        _second_u_old[i] = 0;
        _second_u_older[i] = 0;
      }
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
      RealTensor d2phi_local = _second_phi[i][qp];

      _u[qp]      += phi_local * soln_local;
      _grad_u[qp] += dphi_local * soln_local;
      if (has_second_derivatives)
        _second_u[qp] += d2phi_local * soln_local;

      if (_problem.transient())
      {
        _u_dot[qp]        += phi_local * _sys.solutionUDot()(idx);
        _du_dot_du[qp]    += phi_local * _sys.solutionDuDotDu()(idx);

        _u_old[qp]        += phi_local * soln_old_local;
        _u_older[qp]      += phi_local * soln_older_local;
        _grad_u_old[qp]   += dphi_local * soln_old_local;
        _grad_u_older[qp] += dphi_local * soln_older_local;
        if (has_second_derivatives)
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
  bool has_second_derivatives = (feType().family == CLOUGH || feType().family == HERMITE);
  unsigned int nqp = _qrule_face->n_points();
  _u.resize(nqp);
  _grad_u.resize(nqp);
  if (has_second_derivatives)
    _second_u.resize(nqp);
  if (_problem.transient())
  {
    _u_dot.resize(nqp);
    _du_dot_du.resize(nqp);

    _u_old.resize(nqp);
    _u_older.resize(nqp);
    _grad_u_old.resize(nqp);
    _grad_u_older.resize(nqp);
    if (has_second_derivatives)
    {
      _second_u_old.resize(nqp);
      _second_u_older.resize(nqp);
    }
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
      if (has_second_derivatives)
      {
        _second_u_old[i] = 0;
        _second_u_older[i] = 0;
      }
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
      Real phi_local = _phi_face[i][qp];
      RealGradient dphi_local = _grad_phi_face[i][qp];
      RealTensor d2phi_local = _second_phi_face[i][qp];

      _u[qp]      += phi_local * soln_local;
      _grad_u[qp] += dphi_local * soln_local;
      if (has_second_derivatives)
        _second_u[qp] += d2phi_local * soln_local;

      if (_problem.transient())
      {
        _u_dot[qp]        += phi_local * _sys.solutionUDot()(idx);
        _du_dot_du[qp]    += phi_local * _sys.solutionDuDotDu()(idx);

        _u_old[qp]        += phi_local * soln_old_local;
        _u_older[qp]      += phi_local * soln_older_local;
        _grad_u_old[qp]   += dphi_local * soln_old_local;
        _grad_u_older[qp] += dphi_local * soln_older_local;
        if (has_second_derivatives)
        {
          _second_u_old[qp] += d2phi_local * soln_old_local;
          _second_u_older[qp] += d2phi_local * soln_older_local;
        }
      }
    }
  }
}

void
MooseVariable::computeNodalValues()
{
  _nodal_u.resize(1);
  _nodal_u[0] = _sys.solution()(_nodal_dof_index);

  if (_problem.transient())
  {
    _nodal_u_old.resize(1);
    _nodal_u_old[0] = _sys.solutionOld()(_nodal_dof_index);

    _nodal_u_older.resize(1);
    _nodal_u_older[0] = _sys.solutionOlder()(_nodal_dof_index);
  }
}

Number
MooseVariable::getNodalValue(const Node & node)
{
  unsigned int dof = node.dof_number(_sys.number(), _var_num, 0);
  return _sys.solution()(dof);

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
