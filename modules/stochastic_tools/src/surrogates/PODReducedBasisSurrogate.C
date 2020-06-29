//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PODReducedBasisSurrogate.h"

registerMooseObject("StochasticToolsApp", PODReducedBasisSurrogate);

InputParameters
PODReducedBasisSurrogate::validParams()
{
  InputParameters params = SurrogateModel::validParams();
  params.addClassDescription("Evaluates POD-RB surrogate model with reduced operators "
                             "computed from PODReducedBasisTrainer.");
  return params;
}

PODReducedBasisSurrogate::PODReducedBasisSurrogate(const InputParameters & parameters)
  : SurrogateModel(parameters),
    _var_names(getModelData<std::vector<std::string>>("_var_names")),
    _independent(getModelData<std::vector<unsigned int>>("_independent")),
    _base(getModelData<std::vector<std::vector<DenseVector<Real>>>>("_base")),
    _red_operators(getModelData<std::vector<DenseMatrix<Real>>>("_red_operators")),
    _initialized(false)
{}

void
PODReducedBasisSurrogate::evaluateSolution(const std::vector<Real> & params)
{
  initializeReducedSystem();
  solveReducedSystem(params);
  reconstructApproximateSolution();
}

void
PODReducedBasisSurrogate::initializeReducedSystem()
{
  if(!_initialized)
  {
    _sys_mx = DenseMatrix<Real>(_red_operators[0].m(), _red_operators[0].m());
    _rhs = DenseVector<Real>(_red_operators[0].m());
    _coeffs = DenseVector<Real>(_red_operators[0].m());

    _approx_solution.resize(_var_names.size());
    for(unsigned int var_i=0; var_i<_var_names.size(); var_i++)
    {
      _approx_solution[var_i] = DenseVector<Real>(_base[var_i][0].size());
    }
    _initialized = true;
  }
}

Real
PODReducedBasisSurrogate::getMax(std::string var_name) const
{
  Real val = 0.0;
  auto it = std::find (_var_names.begin(), _var_names.end(), var_name);
  if (it != _var_names.end())
  {
    val = _approx_solution[it-_var_names.begin()].max();
  }
  else
    mooseError("Variable '",var_name,"' not found!");

  return(val);
}

void
PODReducedBasisSurrogate::solveReducedSystem(const std::vector<Real> & params)
{
  // The assumption here is that the reduced operators in the trainer were
  // assembled in the order of the parameters. Also, if the number of
  // parameters is fewer than the number of operators, the operator will
  // just be added without scaling.
  _sys_mx.zero();
  _rhs.zero();
  for(unsigned int i=0; i<params.size(); ++i)
  {
      for(unsigned int row_i=0; row_i<_red_operators[i].m(); row_i++)
        if (!_independent[i])
          for(unsigned int col_i=0; col_i<_red_operators[i].n(); col_i++)
            _sys_mx(row_i, col_i) += params[i] * _red_operators[i](row_i, col_i);
        else
          _rhs(row_i) -= params[i] * _red_operators[i](row_i, 0);
  }

  for(unsigned int i=params.size(); i<_red_operators.size(); ++i)
  {
    if (!_independent[i])
      _sys_mx += _red_operators[i];
    else
      for(unsigned int row_i=0; row_i<_red_operators[i].m(); row_i++)
        _rhs(row_i) -= _red_operators[i](row_i, 0);
  }

  _sys_mx.lu_solve(_rhs, _coeffs);
}

void
PODReducedBasisSurrogate::reconstructApproximateSolution()
{
  unsigned int counter = 0;
  for(unsigned int var_i=0; var_i<_var_names.size(); var_i++)
  {
    _approx_solution[var_i].zero();
    for(unsigned int base_i=0; base_i<_base[var_i].size(); ++base_i)
    {
      for (unsigned int dof_i=0; dof_i<_base[var_i][base_i].size(); ++dof_i)
        _approx_solution[var_i](dof_i) += _coeffs(counter) * _base[var_i][base_i](dof_i);
      counter++;
    }
  }
}

Real
PODReducedBasisSurrogate::evaluate(const std::vector<Real> & /*x*/) const
{
  mooseError("evaluate() is not implemented in PODReducedBasisSurrogate!");
  return 0.0;
}
