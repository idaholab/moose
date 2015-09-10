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

#include "Kernel.h"
#include "Assembly.h"
#include "MooseVariable.h"
#include "Problem.h"
#include "DisplacedProblem.h"
#include "SubProblem.h"
#include "SystemBase.h"

// libmesh includes
#include "libmesh/threads.h"
#include "libmesh/quadrature.h"

template<>
InputParameters validParams<Kernel>()
{
  InputParameters params = validParams<KernelBase>();
  params.registerBase("Kernel");
  params.addParam<std::string>("xfem_qrule", "volfrac", "The type of XFEM quadrature rule");
  return params;
}

Kernel::Kernel(const InputParameters & parameters) :
    KernelBase(parameters),
    _u(_is_implicit ? _var.sln() : _var.slnOld()),
    _grad_u(_is_implicit ? _var.gradSln() : _var.gradSlnOld()),
    _u_dot(_var.uDot()),
    _du_dot_du(_var.duDotDu()),
    _xfem_qrule(getParam<std::string>("xfem_qrule"))
{
}

Kernel::~Kernel()
{
}

void
Kernel::computeResidual()
{
  DenseVector<Number> & re = _assembly.residualBlock(_var.number());
  _local_re.resize(re.size());
  _local_re.zero();
  get_xfem_weights(_xfem_weights); // ZZY

  precalculateResidual();
  for (_i = 0; _i < _test.size(); _i++)
    for (_qp = 0; _qp < _qrule->n_points(); _qp++)
      //_local_re(_i) += _JxW[_qp] * _coord[_qp] * computeQpResidual() * _xfem_weights[_qp];
       _local_re(_i) += _JxW[_qp] * _coord[_qp] * computeQpResidual();

  re += _local_re;

  if (_has_save_in)
  {
    Threads::spin_mutex::scoped_lock lock(Threads::spin_mtx);
    for (unsigned int i=0; i<_save_in.size(); i++)
      _save_in[i]->sys().solution().add_vector(_local_re, _save_in[i]->dofIndices());
  }
}

void
Kernel::computeJacobian()
{
  DenseMatrix<Number> & ke = _assembly.jacobianBlock(_var.number(), _var.number());
  _local_ke.resize(ke.m(), ke.n());
  _local_ke.zero();
  get_xfem_weights(_xfem_weights); // ZZY

  for (_i = 0; _i < _test.size(); _i++)
    for (_j = 0; _j < _phi.size(); _j++)
      for (_qp = 0; _qp < _qrule->n_points(); _qp++)
        //_local_ke(_i, _j) += _JxW[_qp] * _coord[_qp] * computeQpJacobian() * _xfem_weights[_qp];
         _local_ke(_i, _j) += _JxW[_qp] * _coord[_qp] * computeQpJacobian();

  ke += _local_ke;

  if (_has_diag_save_in)
  {
    unsigned int rows = ke.m();
    DenseVector<Number> diag(rows);
    for (unsigned int i=0; i<rows; i++)
      diag(i) = _local_ke(i,i);

    Threads::spin_mutex::scoped_lock lock(Threads::spin_mtx);
    for (unsigned int i=0; i<_diag_save_in.size(); i++)
      _diag_save_in[i]->sys().solution().add_vector(diag, _diag_save_in[i]->dofIndices());
  }
}

void
Kernel::computeOffDiagJacobian(unsigned int jvar)
{
  if (jvar == _var.number())
    computeJacobian();
  else
  {
    DenseMatrix<Number> & ke = _assembly.jacobianBlock(_var.number(), jvar);
    get_xfem_weights(_xfem_weights); // ZZY

    for (_i = 0; _i < _test.size(); _i++)
      for (_j = 0; _j < _phi.size(); _j++)
        for (_qp = 0; _qp < _qrule->n_points(); _qp++)
        {
          //ke(_i, _j) += _JxW[_qp] * _coord[_qp] * computeQpOffDiagJacobian(jvar) * _xfem_weights[_qp];
          ke(_i, _j) += _JxW[_qp] * _coord[_qp] * computeQpOffDiagJacobian(jvar);
        }
  }
}

void
Kernel::computeOffDiagJacobianScalar(unsigned int jvar)
{
  DenseMatrix<Number> & ke = _assembly.jacobianBlock(_var.number(), jvar);
  MooseVariableScalar & jv = _sys.getScalarVariable(_tid, jvar);
  get_xfem_weights(_xfem_weights); // ZZY

  for (_i = 0; _i < _test.size(); _i++)
    for (_j = 0; _j < jv.order(); _j++)
      for (_qp = 0; _qp < _qrule->n_points(); _qp++)
        //ke(_i, _j) += _JxW[_qp] * _coord[_qp] * computeQpOffDiagJacobian(jvar) * _xfem_weights[_qp];
        ke(_i, _j) += _JxW[_qp] * _coord[_qp] * computeQpOffDiagJacobian(jvar) * _xfem_weights[_qp];
}

Real
Kernel::computeQpJacobian()
{
  return 0;
}

Real
Kernel::computeQpOffDiagJacobian(unsigned int /*jvar*/)
{
  return 0;
}

void
Kernel::precalculateResidual()
{
}

void
Kernel::get_xfem_weights(std::vector<Real> & _xfem_weights)
{
  XFEM* xfem = _fe_problem.get_xfem();
  _xfem_weights.resize(_qrule->n_points(), 1.0);

  const Elem * undisplaced_elem  = NULL;
  if (getParam<bool>("use_displaced_mesh") && _fe_problem.getDisplacedProblem() != NULL)
  {
    DisplacedProblem & displaced_problem = dynamic_cast< DisplacedProblem & >(_subproblem);
    undisplaced_elem = displaced_problem.refMesh().elem(_current_elem->id());
  }
  else
    undisplaced_elem = _current_elem;

  for (_qp = 0; _qp < _qrule->n_points(); _qp++)
  {
    if (_xfem_qrule == "volfrac")
      _xfem_weights[_qp] = xfem->get_elem_phys_volfrac(undisplaced_elem);
    else if (_xfem_qrule == "moment_fitting"){
      std::vector<Point> gauss_points = _qrule->get_points();
      std::vector<Real>  gauss_weights = _qrule->get_weights();
      _xfem_weights[_qp] = xfem->get_elem_new_weights(undisplaced_elem, _qp, gauss_points, gauss_weights);
      //std::vector<Point> test_points = _qrule->get_points();
      //for(int i = 0; i < test_points.size(); i++){
      //  std::cout << "[" << i << "] point = " << test_points[i] << std::endl;
      //}
    }
    else if (_xfem_qrule == "direct") // remove q-points outside the elem real domain by force
      _xfem_weights[_qp] = xfem->flag_qp_inside(undisplaced_elem, _q_point[_qp]);
    else
      mooseError("unrecognized XFEM qrule option!");
  }
}
