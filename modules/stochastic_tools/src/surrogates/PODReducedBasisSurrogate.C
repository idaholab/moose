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
  params.addParam<std::vector<std::string>>("change_rank",
                                            std::vector<std::string>(0),
                                            "Names of variables whose rank should be changed.");
  params.addParam<std::vector<unsigned int>>(
      "new_ranks",
      std::vector<unsigned int>(0),
      "The new ranks that each variable in 'change_rank' shall have.");
  params.addParam<Real>("penalty", 1e5, "The penalty parameter for Dirichlet BCs.");
  return params;
}

PODReducedBasisSurrogate::PODReducedBasisSurrogate(const InputParameters & parameters)
  : SurrogateModel(parameters),
    _change_rank(getParam<std::vector<std::string>>("change_rank")),
    _new_ranks(getParam<std::vector<unsigned int>>("new_ranks")),
    _var_names(getModelData<std::vector<std::string>>("_var_names")),
    _tag_types(getModelData<std::vector<std::string>>("_tag_types")),
    _base(getModelData<std::vector<std::vector<DenseVector<Real>>>>("_base")),
    _red_operators(getModelData<std::vector<DenseMatrix<Real>>>("_red_operators")),
    _penalty(getParam<Real>("penalty")),
    _initialized(false)
{
  if (_change_rank.size() != _new_ranks.size())
    paramError("new_ranks",
               "The size of 'new_ranks' is not equal to the ",
               "size of 'change_rank' ",
               _new_ranks.size(),
               " != ",
               _change_rank.size());

  for (unsigned int var_i = 0; var_i < _new_ranks.size(); ++var_i)
    if (_new_ranks[var_i] == 0)
      paramError("new_ranks", "The values should be greater than 0!");
}

void
PODReducedBasisSurrogate::evaluateSolution(const std::vector<Real> & params)
{
  if (!_initialized)
  {
    // The containers are initialized (if needed).
    initializeReducedSystem();
    initializeApproximateSolution();
    _initialized = true;
  }

  // Assembling and solving the reduced equation system.
  solveReducedSystem(params);

  // Reconstructing the approximate solutions for every variable.
  reconstructApproximateSolution();
}

void
PODReducedBasisSurrogate::evaluateSolution(const std::vector<Real> & params,
                                           DenseVector<Real> & inp_vector,
                                           std::string var_name)
{
  if (!_initialized)
  {
    // The containers are initialized (if needed).
    initializeReducedSystem();
    _initialized = true;
  }

  // Assembling and solving the reduced equation system.
  solveReducedSystem(params);

  // Reconstructing the approximate solutions for every variable.
  reconstructApproximateSolution(inp_vector, var_name);
}

void
PODReducedBasisSurrogate::initializeReducedSystem()
{
  // Storing important indices for the assembly loops.
  _final_ranks.resize(_var_names.size());
  _comulative_ranks.resize(_var_names.size());
  unsigned int sum_ranks = 0;

  // Checking if the user wants to overwrite the original ranks for the
  // variables.
  for (unsigned int var_i = 0; var_i < _var_names.size(); ++var_i)
  {
    _final_ranks[var_i] = _base[var_i].size();
    for (unsigned int var_j = 0; var_j < _change_rank.size(); ++var_j)
    {
      if (_change_rank[var_j] == _var_names[var_i])
      {
        if (_new_ranks[var_j] > _base[var_i].size())
        {
          mooseWarning("The specified new rank (",
                       _new_ranks[var_j],
                       ") for variable '",
                       _var_names[var_i],
                       "' is higher than the original rank (",
                       _base[var_i].size(),
                       ")! Switched to original rank.");
          break;
        }

        _final_ranks[var_i] = _new_ranks[var_j];
      }
    }
    sum_ranks += _final_ranks[var_i];
    _comulative_ranks[var_i] = sum_ranks;
  }

  // Resizing containers to match the newly prescribed ranks.
  _sys_mx = DenseMatrix<Real>(sum_ranks, sum_ranks);
  _rhs = DenseVector<Real>(sum_ranks);
  _coeffs = DenseVector<Real>(sum_ranks);
}

void
PODReducedBasisSurrogate::initializeApproximateSolution()
{
  _approx_solution.resize(_var_names.size());
  for (unsigned int var_i = 0; var_i < _var_names.size(); var_i++)
    _approx_solution[var_i] = DenseVector<Real>(_base[var_i][0].size());
}

void
PODReducedBasisSurrogate::solveReducedSystem(const std::vector<Real> & params)
{
  // Cleaning the containers of the system matrix and right hand side.
  _sys_mx.zero();
  _rhs.zero();

  // The assumption here is that the reduced operators in the trainer were
  // assembled in the order of the parameters. Also, if the number of
  // parameters is fewer than the number of operators, the operator will
  // just be added without scaling.
  for (unsigned int i = 0; i < _red_operators.size(); ++i)
  {
    unsigned int row_start = 0;

    // Checking if the reduced operator corresponds to a Dirichlet BC, if
    // yes introduce the penalty factor.
    Real factor = 1.0;
    if (_tag_types[i] == "op_dir" || _tag_types[i] == "src_dir")
      factor = _penalty;

    // If the user decreased the rank of the reduced bases manually, some parts
    // of the initial reduced operators have to be omited.
    for (unsigned int var_i = 0; var_i < _var_names.size(); ++var_i)
    {
      for (unsigned int row_i = row_start; row_i < _comulative_ranks[var_i]; row_i++)
      {
        if (_tag_types[i] == "op" || _tag_types[i] == "op_dir")
        {

          unsigned int col_start = 0;

          for (unsigned int var_j = 0; var_j < _var_names.size(); ++var_j)
          {
            for (unsigned int col_i = col_start; col_i < _comulative_ranks[var_j]; col_i++)
            {
              if (i < params.size())
                _sys_mx(row_i, col_i) += params[i] * factor * _red_operators[i](row_i, col_i);
              else
                _sys_mx(row_i, col_i) += factor * _red_operators[i](row_i, col_i);
            }

            col_start = _comulative_ranks[var_j];
          }
        }
        else
        {
          if (i < params.size())
            _rhs(row_i) -= params[i] * factor * _red_operators[i](row_i, 0);
          else
            _rhs(row_i) -= factor * _red_operators[i](row_i, 0);
        }
        row_start = _comulative_ranks[var_i];
      }
    }
  }

  // Solving the reduced system.
  _sys_mx.lu_solve(_rhs, _coeffs);
}

void
PODReducedBasisSurrogate::reconstructApproximateSolution()
{
  unsigned int counter = 0;
  for (unsigned int var_i = 0; var_i < _var_names.size(); var_i++)
  {
    _approx_solution[var_i].zero();

    // This also takes into account the potential truncation of the bases by
    // the user.
    for (unsigned int base_i = 0; base_i < _final_ranks[var_i]; ++base_i)
    {
      for (unsigned int dof_i = 0; dof_i < _base[var_i][base_i].size(); ++dof_i)
        _approx_solution[var_i](dof_i) += _coeffs(counter) * _base[var_i][base_i](dof_i);
      counter++;
    }
  }
}

void
PODReducedBasisSurrogate::reconstructApproximateSolution(DenseVector<Real> & inp_vector,
                                                         std::string var_name)
{
  auto it = std::find(_var_names.begin(), _var_names.end(), var_name);
  if (it == _var_names.end())
    mooseError("Variable '", var_name, "' does not exist in the POD-RB surrogate!");

  unsigned int var_i = std::distance(_var_names.begin(), it);

  if (inp_vector.size() != _base[var_i][0].size())
    mooseError("The size of the input vector (",
               inp_vector.size(),
               ") for variable '",
               var_name,
               "' does not match the size of the stored base vector (",
               _base[var_i][0].size(),
               ") in POD-RB surrogate!");

  inp_vector.zero();

  unsigned int base_begin = 0;
  if (var_i != 0)
    base_begin = _comulative_ranks[var_i - 1];

  for (unsigned int base_i = 0; base_i < _final_ranks[var_i]; ++base_i)
  {
    for (unsigned int dof_i = 0; dof_i < inp_vector.size(); ++dof_i)
      inp_vector(dof_i) += _coeffs(base_i + base_begin) * _base[var_i][base_i](dof_i);
  }
}

Real
PODReducedBasisSurrogate::getNodalQoI(std::string var_name, unsigned int qoi_type) const
{
  Real val = 0.0;

  auto it = std::find(_var_names.begin(), _var_names.end(), var_name);
  if (it == _var_names.end())
    mooseError("Variable '", var_name, "' not found!");

  switch (qoi_type)
  {
    case 0:
      val = _approx_solution[it - _var_names.begin()].max();
      break;

    case 1:
      val = _approx_solution[it - _var_names.begin()].min();
      break;

    case 2:
      val = _approx_solution[it - _var_names.begin()].l1_norm();
      break;

    case 3:
      val = _approx_solution[it - _var_names.begin()].l2_norm();
      break;

    case 4:
      val = _approx_solution[it - _var_names.begin()].linfty_norm();
      break;
  }

  return (val);
}
