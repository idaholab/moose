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

// MOOSE includes
#include "Assembly.h"
#include "MooseVariable.h"
#include "MooseVariableScalar.h"
#include "Problem.h"
#include "SubProblem.h"
#include "SystemBase.h"

#include "libmesh/threads.h"
#include "libmesh/quadrature.h"

template <>
InputParameters
validParams<Kernel>()
{
  InputParameters params = validParams<KernelBase>();
  params.registerBase("Kernel");
  return params;
}

Kernel::Kernel(const InputParameters & parameters)
  : KernelBase(parameters),
    _u(_is_implicit ? _var.sln() : _var.slnOld()),
    _grad_u(_is_implicit ? _var.gradSln() : _var.gradSlnOld()),
    _u_dot(_var.uDot()),
    _du_dot_du(_var.duDotDu())
{
  auto & vector_tag_names = getParam<MultiMooseEnum>("vector_tags");

  if (!vector_tag_names.isValid())
    mooseError("MUST provide at least one vector_tag for Kernel: ", name());

  for (auto & vector_tag_name : vector_tag_names)
  {
    if (!_fe_problem.vectorTagExists(vector_tag_name))
      mooseError("Kernel, ",
                 name(),
                 ", was assigned an invalid vector_tag: '",
                 vector_tag_name,
                 "'.  If this is a TimeKernel then this may have happened because you didn't "
                 "specify a Transient Executioner.");

    _vector_tags.push_back(_fe_problem.getVectorTag(vector_tag_name));
    std::cout << name() << " Kernel using vector tag: " << _vector_tags.back() << std::endl;
  }

  auto & matrix_tag_names = getParam<MultiMooseEnum>("matrix_tags");

  if (!matrix_tag_names.isValid())
    mooseError("MUST provide at least one matrix_tag for Kernel: ", name());

  for (auto & matrix_tag_name : matrix_tag_names)
  {
    if (!_fe_problem.matrixTagExists(matrix_tag_name))
      mooseError("Kernel, ",
                 name(),
                 ", was assigned an invalid matrix_tag: '",
                 matrix_tag_name,
                 "'.  If this is a TimeKernel then this may have happened because you didn't "
                 "specify a Transient Executioner.");

    _matrix_tags.push_back(_fe_problem.getMatrixTag(matrix_tag_name));
    std::cout << name() << " Kernel using matrix tag: " << _matrix_tags.back() << std::endl;
  }

  _re_blocks.resize(_vector_tags.size());
  _ke_blocks.resize(_matrix_tags.size());
}

void
Kernel::computeResidual()
{
  for (auto i = beginIndex(_vector_tags); i < _vector_tags.size(); i++)
    _re_blocks[i] = &_assembly.residualBlock(_var.number(), _vector_tags[i]);

  _local_re.resize(_re_blocks[0]->size());
  _local_re.zero();

  precalculateResidual();
  for (_i = 0; _i < _test.size(); _i++)
    for (_qp = 0; _qp < _qrule->n_points(); _qp++)
      _local_re(_i) += _JxW[_qp] * _coord[_qp] * computeQpResidual();

  for (auto & re : _re_blocks)
    *re += _local_re;

  if (_has_save_in)
  {
    Threads::spin_mutex::scoped_lock lock(Threads::spin_mtx);
    for (const auto & var : _save_in)
      var->sys().solution().add_vector(_local_re, var->dofIndices());
  }
}

void
Kernel::computeJacobian()
{
  for (auto i = beginIndex(_matrix_tags); i < _matrix_tags.size(); i++)
    _ke_blocks[i] = &_assembly.jacobianBlock(_var.number(), _var.number(), _matrix_tags[i]);

  _local_ke.resize(_ke_blocks[0]->m(), _ke_blocks[0]->n());
  _local_ke.zero();

  precalculateJacobian();
  for (_i = 0; _i < _test.size(); _i++)
    for (_j = 0; _j < _phi.size(); _j++)
      for (_qp = 0; _qp < _qrule->n_points(); _qp++)
        _local_ke(_i, _j) += _JxW[_qp] * _coord[_qp] * computeQpJacobian();

  for (auto & ke : _ke_blocks)
    *ke += _local_ke;

  if (_has_diag_save_in)
  {
    unsigned int rows = _local_ke.m();
    DenseVector<Number> diag(rows);
    for (unsigned int i = 0; i < rows; i++)
      diag(i) = _local_ke(i, i);

    Threads::spin_mutex::scoped_lock lock(Threads::spin_mtx);
    for (const auto & var : _diag_save_in)
      var->sys().solution().add_vector(diag, var->dofIndices());
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

    precalculateOffDiagJacobian(jvar);
    for (_i = 0; _i < _test.size(); _i++)
      for (_j = 0; _j < _phi.size(); _j++)
        for (_qp = 0; _qp < _qrule->n_points(); _qp++)
          ke(_i, _j) += _JxW[_qp] * _coord[_qp] * computeQpOffDiagJacobian(jvar);
  }
}

void
Kernel::computeOffDiagJacobianScalar(unsigned int jvar)
{
  DenseMatrix<Number> & ke = _assembly.jacobianBlock(_var.number(), jvar);
  MooseVariableScalar & jv = _sys.getScalarVariable(_tid, jvar);

  for (_i = 0; _i < _test.size(); _i++)
    for (_j = 0; _j < jv.order(); _j++)
      for (_qp = 0; _qp < _qrule->n_points(); _qp++)
        ke(_i, _j) += _JxW[_qp] * _coord[_qp] * computeQpOffDiagJacobian(jvar);
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
