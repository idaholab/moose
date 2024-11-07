//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ChainControlParsedFunctionWrapper.h"
#include "MooseApp.h"
#include "FEProblem.h"
#include "MooseVariableScalar.h"
#include "Function.h"
#include "MooseUtils.h"
#include "ChainControlDataSystem.h"

ChainControlParsedFunctionWrapper::ChainControlParsedFunctionWrapper(
    MooseApp & moose_app,
    FEProblemBase & fe_problem,
    const std::string & function_str,
    const std::vector<std::string> & symbol_names,
    const std::vector<std::string> & symbol_values,
    const THREAD_ID tid)
  : _moose_app(moose_app),
    _fe_problem(fe_problem),
    _function_str(function_str),
    _symbol_names(symbol_names),
    _symbol_values(symbol_values),
    _tid(tid),
    _chain_control_data_system(_moose_app.getChainControlDataSystem())
{
  initializeFunctionInputs();

  _function_ptr = std::make_unique<ParsedFunction<Real, RealGradient>>(
      _function_str, &_symbol_names, &_initial_values);

  for (const auto & symbol_name : _symbol_names)
    _input_values.push_back(&_function_ptr->getVarAddress(symbol_name));
}

Real
ChainControlParsedFunctionWrapper::evaluate(Real t, const Point & p)
{
  updateScalarVariableValues();
  updateFunctionValues(t, p);
  updateChainControlDataValues();
  return (*_function_ptr)(p, t);
}

void
ChainControlParsedFunctionWrapper::initializeFunctionInputs()
{
  for (const auto i : index_range(_symbol_values))
  {
    if (_chain_control_data_system.hasChainControlDataOfType<Real>(_symbol_values[i]))
    {
      auto & data = _chain_control_data_system.getChainControlData<Real>(_symbol_values[i]);
      _initial_values.push_back(data.get());
      _real_control_data_values.push_back(&data);
      _real_control_data_indices.push_back(i);
    }
    else if (_chain_control_data_system.hasChainControlDataOfType<bool>(_symbol_values[i]))
    {
      auto & data = _chain_control_data_system.getChainControlData<bool>(_symbol_values[i]);
      _initial_values.push_back(data.get());
      _bool_control_data_values.push_back(&data);
      _bool_control_data_indices.push_back(i);
    }
    else if (_fe_problem.hasScalarVariable(_symbol_values[i]))
    {
      const VariableValue & scalar_val =
          _fe_problem.getScalarVariable(_tid, _symbol_values[i]).sln();
      _initial_values.push_back(0);
      _scalar_values.push_back(&scalar_val);
      _scalar_indices.push_back(i);
    }
    else if (_fe_problem.hasFunction(_symbol_values[i]))
    {
      const Function & function = _fe_problem.getFunction(_symbol_values[i], _tid);
      _initial_values.push_back(0);
      _function_values.push_back(&function);
      _function_indices.push_back(i);
    }
    else if (MooseUtils::isFloat(_symbol_values[i]))
    {
      _initial_values.push_back(MooseUtils::convert<Real>(_symbol_values[i], true));
    }
    else
      mooseError("Invalid 'symbol_values' entry '",
                 _symbol_values[i],
                 "'. Valid entries:\n"
                 "  - chain control data values (bool or Real)\n"
                 "  - function names\n"
                 "  - scalar variable names\n"
                 "  - constant values");
  }
}

void
ChainControlParsedFunctionWrapper::updateScalarVariableValues()
{
  for (const auto i : index_range(_scalar_indices))
    (*_input_values[_scalar_indices[i]]) = (*_scalar_values[i])[0];
}

void
ChainControlParsedFunctionWrapper::updateFunctionValues(Real t, const Point & pt)
{
  for (const auto i : index_range(_function_indices))
    (*_input_values[_function_indices[i]]) = _function_values[i]->value(t, pt);
}

void
ChainControlParsedFunctionWrapper::updateChainControlDataValues()
{
  for (const auto i : index_range(_real_control_data_indices))
    (*_input_values[_real_control_data_indices[i]]) = _real_control_data_values[i]->get();

  for (const auto i : index_range(_bool_control_data_indices))
    (*_input_values[_bool_control_data_indices[i]]) =
        static_cast<Real>(_bool_control_data_values[i]->get());
}
