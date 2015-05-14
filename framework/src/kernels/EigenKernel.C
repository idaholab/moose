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

#include "EigenKernel.h"
#include "EigenSystem.h"
#include "MooseApp.h"
#include "Executioner.h"
#include "EigenExecutionerBase.h"

template<>
InputParameters validParams<EigenKernel>()
{
  InputParameters params = validParams<KernelBase>();
  params.addParam<bool>("eigen", true, "Use for eigenvalue problem (true) or source problem (false)");
  params.addParam<PostprocessorName>("eigen_postprocessor", 1.0, "The name of the postprocessor that provides the eigenvalue.");
  params.registerBase("EigenKernel");
  return params;
}

EigenKernel::EigenKernel(const std::string & name, InputParameters parameters) :
    KernelBase(name, parameters),
    _u(_is_implicit ? _var.sln() : _var.slnOld()),
    _grad_u(_is_implicit ? _var.gradSln() : _var.gradSlnOld()),
    _eigen(getParam<bool>("eigen")),
    _eigen_sys(dynamic_cast<EigenSystem *>(&_fe_problem.getNonlinearSystem()))//,
    //  _eigenvalue(_is_implicit ? &getPostprocessorValue("eigen_postprocessor") :
    //            &getPostprocessorValueOld("eigen_postprocessor"))
{
}


void
EigenKernel::initialSetup()
{


  std::string pp_name;

  // If the PP exists, use it. isParamValid does not work here because of the default value, which
  // you don't want to use if an EigenExecutioner exists.
  if (hasPostprocessor("eigen_postprocessor"))
    pp_name = getParam<PostprocessorName>("eigen_postprocessor");


  else
  {
    EigenExecutionerBase * exec = dynamic_cast<EigenExecutionerBase *>(_app.getExecutioner());

    if (exec)
      pp_name = exec->getParam<PostprocessorName>("bx_norm");
/*
    else
    {
      pp_name = "eigenvalue";

      if (!_fe_problem.hasPostprocessor(pp_name))
      {

        InputParameters pp_params = _app.getFactory().getValidParams("EigenValueReporter");
        pp_params.set<MultiMooseEnum>("execute_on") = "initial timestep_end";
        pp_params.set<std::vector<OutputName> >("outputs") = std::vector<OutputName>(1, "none");
        _fe_problem.addPostprocessor("EigenValueReporter", pp_name, pp_params);
      }
    }
*/
  }

  std::cout << "pp_name = " << pp_name << std::endl;


  if (pp_name.empty() && parameters().hasDefaultPostprocessorValue("eigen_postprocessor"))
    _eigenvalue = &parameters().defaultPostprocessorValue("eigen_postprocessor");

  else if (pp_name.empty())
    mooseError("Failed to determine proper pp for " << name());

  else
  {

  if (_is_implicit)
    _eigenvalue = &getPostprocessorValueByName(pp_name);
  else
    _eigenvalue = &getPostprocessorValueOldByName(pp_name);
  }

  std::cout << "_eigenvalue = " << *_eigenvalue << std::endl;
}

const Real &
EigenKernel::eigenvalue()
{
  return *_eigenvalue;
}


void
EigenKernel::computeResidual()
{

  DenseVector<Number> & re = _assembly.residualBlock(_var.number());
  _local_re.resize(re.size());
  _local_re.zero();

  Real one_over_eigen = 1.0;
  one_over_eigen /= eigenvalue();
  for (_i = 0; _i < _test.size(); _i++)
    for (_qp = 0; _qp < _qrule->n_points(); _qp++)
      _local_re(_i) += _JxW[_qp] * _coord[_qp] * one_over_eigen * computeQpResidual();

  re += _local_re;

  if (_has_save_in)
  {
    Threads::spin_mutex::scoped_lock lock(Threads::spin_mtx);
    for (unsigned int i=0; i<_save_in.size(); i++)
      _save_in[i]->sys().solution().add_vector(_local_re, _save_in[i]->dofIndices());
  }
}

void
EigenKernel::computeJacobian()
{
  if (!_is_implicit) return;

  DenseMatrix<Number> & ke = _assembly.jacobianBlock(_var.number(), _var.number());
  _local_ke.resize(ke.m(), ke.n());
  _local_ke.zero();

  Real one_over_eigen = 1.0;
  one_over_eigen /= eigenvalue();
  for (_i = 0; _i < _test.size(); _i++)
    for (_j = 0; _j < _phi.size(); _j++)
      for (_qp = 0; _qp < _qrule->n_points(); _qp++)
        _local_ke(_i, _j) += _JxW[_qp] * _coord[_qp] * one_over_eigen * computeQpJacobian();

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

Real
EigenKernel::computeQpJacobian()
{
  return 0;
}

bool
EigenKernel::isActive()
{
  bool flag = TransientInterface::isActive();
  if (_eigen)
  {
    if (_is_implicit)
      return flag && (!_eigen_sys->activeOnOld());
    else
      return flag && _eigen_sys->activeOnOld();
  }
  else
    return flag;
}
