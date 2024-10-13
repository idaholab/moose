//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "THMParsedFunctionWrapper.h"
#include "Simulation.h"
#include "FEProblem.h"
#include "MooseVariableScalar.h"
#include "Function.h"
#include "MooseUtils.h"

THMParsedFunctionWrapper::THMParsedFunctionWrapper(Simulation & sim,
                                                   FEProblemBase & feproblem,
                                                   const std::string & function_str,
                                                   const std::vector<std::string> & vars,
                                                   const std::vector<std::string> & vals,
                                                   const THREAD_ID tid)
  : _sim(sim),
    _feproblem(feproblem),
    _function_str(function_str),
    _vars(vars),
    _vals_input(vals),
    _tid(tid)
{
  initialize();

  _function_ptr = std::make_unique<libMesh::ParsedFunction<Real, RealGradient>>(
      _function_str, &_vars, &_initial_vals);

  for (auto & v : _vars)
    _addr.push_back(&_function_ptr->getVarAddress(v));
}

Real
THMParsedFunctionWrapper::evaluate(Real t, const Point & p)
{
  update();
  updateFunctionValues(t, p);
  updateControlDataValues();
  return (*_function_ptr)(p, t);
}

void
THMParsedFunctionWrapper::initialize()
{
  for (unsigned int i = 0; i < _vals_input.size(); ++i)
  {
    if (_sim.hasControlData<Real>(_vals_input[i]))
    {
      ControlData<Real> * cd_val = _sim.getControlData<Real>(_vals_input[i]);
      _initial_vals.push_back(cd_val->get());
      _cd_real_vals.push_back(cd_val);
      _cd_real_index.push_back(i);
    }
    else if (_sim.hasControlData<bool>(_vals_input[i]))
    {
      ControlData<bool> * cd_val = _sim.getControlData<bool>(_vals_input[i]);
      _initial_vals.push_back(cd_val->get());
      _cd_bool_vals.push_back(cd_val);
      _cd_bool_index.push_back(i);
    }
    else if (_feproblem.hasScalarVariable(_vals_input[i]))
    {
      const VariableValue & scalar_val = _feproblem.getScalarVariable(_tid, _vals_input[i]).sln();
      _initial_vals.push_back(0);
      _scalar_vals.push_back(&scalar_val);
      _scalar_index.push_back(i);
    }
    else if (_feproblem.hasFunction(_vals_input[i]))
    {
      const Function & fn = _feproblem.getFunction(_vals_input[i], _tid);
      _initial_vals.push_back(0);
      _functions.push_back(&fn);
      _function_index.push_back(i);
    }
    else
    {
      Real val = MooseUtils::convert<Real>(_vals_input[i], true);
      _initial_vals.push_back(val);
    }
  }
}

void
THMParsedFunctionWrapper::update()
{
  for (unsigned int i = 0; i < _scalar_index.size(); ++i)
    (*_addr[_scalar_index[i]]) = (*_scalar_vals[i])[0];
}

void
THMParsedFunctionWrapper::updateFunctionValues(Real t, const Point & pt)
{
  for (unsigned int i = 0; i < _function_index.size(); ++i)
    (*_addr[_function_index[i]]) = _functions[i]->value(t, pt);
}

void
THMParsedFunctionWrapper::updateControlDataValues()
{
  for (unsigned int i = 0; i < _cd_real_index.size(); ++i)
    (*_addr[_cd_real_index[i]]) = _cd_real_vals[i]->get();

  for (unsigned int i = 0; i < _cd_bool_index.size(); ++i)
    (*_addr[_cd_bool_index[i]]) = static_cast<Real>(_cd_bool_vals[i]->get());
}
