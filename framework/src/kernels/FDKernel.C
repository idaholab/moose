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

#include "FDKernel.h"
#include "Assembly.h"
#include "MooseVariable.h"
#include "Problem.h"
#include "SubProblem.h"
#include "SystemBase.h"

// libmesh includes
#include "libmesh/threads.h"

template<>
InputParameters validParams<FDKernel>()
{
  InputParameters params = validParams<Kernel>();
  return params;
}

FDKernel::FDKernel(const std::string & name, InputParameters parameters) :
  Kernel(name, parameters)
{
    _scale = 1.490116119384766e-08; // HACK: sqrt of the machine epsilon for double precision
#ifdef LIBMESH_HAVE_PETSC
    _scale = PETSC_SQRT_MACHINE_EPSILON;
#endif
}

DenseVector<Number>
FDKernel::perturbedResidual(unsigned int varnum, unsigned int perturbationj, Real perturbation_scale, Real& perturbation)
{
  DenseVector<Number> re;
  re.resize(_var.dofIndices().size());
  re.zero();

  MooseVariable& var = _sys.getVariable(_tid,varnum);
  var.computePerturbedElemValues(perturbationj,perturbation_scale,perturbation);
  precalculateResidual();
  for (_i = 0; _i < _test.size(); _i++)
    for (_qp = 0; _qp < _qrule->n_points(); _qp++)
      re(_i) += _JxW[_qp] * _coord[_qp] * computeQpResidual();
  var.restoreUnperturbedElemValues();
  return re;
}

void
FDKernel::computeJacobian()
{
  computeOffDiagJacobian(_var.index());
}

void
FDKernel::computeOffDiagJacobian(unsigned int jvar_index)
{

  DenseMatrix<Number> & ke = _assembly.jacobianBlock(_var.index(), jvar_index);
  DenseMatrix<Number> local_ke;
  local_ke.resize(ke.m(), ke.n());
  local_ke.zero();

  // FIXME: pull out the already computed element residual instead of recomputing it
  Real h;
  DenseVector<Number> re = perturbedResidual(_var.index(),0,0.0,h);
  for (_j = 0; _j < _phi.size(); _j++) {
    DenseVector<Number> p_re = perturbedResidual(jvar_index,_j,_scale,h);
    for(_i = 0; _i < _test.size(); _i++) {
      local_ke(_i,_j) = (p_re(_i) - re(_i))/h;
    }
  }
  ke += local_ke;

  if (jvar_index == _var.index()) {
    _local_ke = local_ke;
    if (_has_diag_save_in) {
      unsigned int rows = ke.m();
      DenseVector<Number> diag(rows);
      for(unsigned int i=0; i<rows; i++)
	diag(i) = _local_ke(i,i);

      Threads::spin_mutex::scoped_lock lock(Threads::spin_mtx);
      for(unsigned int i=0; i<_diag_save_in.size(); i++)
	_diag_save_in[i]->sys().solution().add_vector(diag, _diag_save_in[i]->dofIndices());
    }
  }
}

void
FDKernel::computeOffDiagJacobianScalar(unsigned int /*jvar*/)
{
  // FIXME: implement me.
  /*
  DenseMatrix<Number> & ke = _assembly.jacobianBlock(_var.index(), jvar);
  MooseVariableScalar & jv = _sys.getScalarVariable(_tid, jvar);

  for (_i = 0; _i < _test.size(); _i++)
    for (_j = 0; _j < jv.order(); _j++)
      for (_qp = 0; _qp < _qrule->n_points(); _qp++)
        ke(_i, _j) += _JxW[_qp] * _coord[_qp] * computeQpOffDiagJacobian(jvar);
  */
}

