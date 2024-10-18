//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MooseParsedFunctionWrapper.h"
#include "FEProblem.h"
#include "MooseVariableScalar.h"
#include "Function.h"
#include "MooseUtils.h"

using namespace libMesh;

MooseParsedFunctionWrapper::MooseParsedFunctionWrapper(FEProblemBase & feproblem,
                                                       const std::string & function_str,
                                                       const std::vector<std::string> & vars,
                                                       const std::vector<std::string> & vals,
                                                       const THREAD_ID tid)
  : _feproblem(feproblem), _function_str(function_str), _vars(vars), _vals_input(vals), _tid(tid)
{
  initialize();

  _function_ptr =
      std::make_unique<ParsedFunction<Real, RealGradient>>(_function_str, &_vars, &_initial_vals);

  for (auto & v : _vars)
    _addr.push_back(&_function_ptr->getVarAddress(v));
}

MooseParsedFunctionWrapper::~MooseParsedFunctionWrapper() {}

template <>
Real
MooseParsedFunctionWrapper::evaluate(Real t, const Point & p)
{
  update();
  updateFunctionValues(t, p);
  return (*_function_ptr)(p, t);
}

template <>
DenseVector<Real>
MooseParsedFunctionWrapper::evaluate(Real t, const Point & p)
{
  update();
  updateFunctionValues(t, p);
  DenseVector<Real> output(LIBMESH_DIM);
  (*_function_ptr)(p, t, output);
  return output;
}

template <>
RealVectorValue
MooseParsedFunctionWrapper::evaluate(Real t, const Point & p)
{
  DenseVector<Real> output = evaluate<DenseVector<Real>>(t, p);

  return RealVectorValue(output(0)
#if LIBMESH_DIM > 1
                             ,
                         output(1)
#endif
#if LIBMESH_DIM > 2
                             ,
                         output(2)
#endif
  );
}

RealGradient
MooseParsedFunctionWrapper::evaluateGradient(Real t, const Point & p)
{
  update();
  updateFunctionValues(t, p);
  return _function_ptr->gradient(p, t);
}

Real
MooseParsedFunctionWrapper::evaluateDot(Real t, const Point & p)
{
  update();
  updateFunctionValues(t, p);
  return _function_ptr->dot(p, t);
}

void
MooseParsedFunctionWrapper::initialize()
{
  // Loop through all the input values supplied by the users.
  for (unsigned int i = 0; i < _vals_input.size(); ++i)
  {
    // Case when a Postprocessor is found by the name given in the input values
    ReporterName r_name(_vals_input[i], "value");
    if (_feproblem.getReporterData().hasReporterValue<PostprocessorValue>(r_name))
    {
      const Real & pp_val = _feproblem.getPostprocessorValueByName(_vals_input[i]);
      _initial_vals.push_back(pp_val);
      _pp_vals.push_back(&pp_val);
      _pp_index.push_back(i);
    }

    // Case when a scalar variable is found by the name given in the input values
    else if (_feproblem.hasScalarVariable(_vals_input[i]))
    {
      auto & scalar_val = _feproblem.getScalarVariable(_tid, _vals_input[i]).sln()[0];
      _initial_vals.push_back(scalar_val);
      _scalar_vals.push_back(&scalar_val);
      _scalar_index.push_back(i);
    }

    // Case when a function is found by the name given in the input values
    else if (_feproblem.hasFunction(_vals_input[i]))
    {
      Function & fn = _feproblem.getFunction(_vals_input[i], _tid);
      _initial_vals.push_back(0);
      _functions.push_back(&fn);
      _function_index.push_back(i);
    }

    // Case when a Real is supplied
    else
    {
      Real val;
      try
      {
        val = MooseUtils::convert<Real>(_vals_input[i], true);
      }
      catch (const std::invalid_argument & e)
      {
        mooseError("'No postprocessor, scalar variable, or function with the name '",
                   _vals_input[i],
                   "' found. ",
                   e.what());
      }
      _initial_vals.push_back(val);
    }
  }
}

void
MooseParsedFunctionWrapper::update()
{
  for (unsigned int i = 0; i < _pp_index.size(); ++i)
    (*_addr[_pp_index[i]]) = (*_pp_vals[i]);

  for (unsigned int i = 0; i < _scalar_index.size(); ++i)
    (*_addr[_scalar_index[i]]) = (*_scalar_vals[i]);
}

void
MooseParsedFunctionWrapper::updateFunctionValues(Real t, const Point & pt)
{
  for (unsigned int i = 0; i < _function_index.size(); ++i)
    (*_addr[_function_index[i]]) = _functions[i]->value(t, pt);
}
