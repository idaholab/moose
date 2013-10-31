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

#include "MooseParsedFunctionWrapper.h"

MooseParsedFunctionWrapper::MooseParsedFunctionWrapper(FEProblem & feproblem,
                                                     const std::string & function_str,
                                                     const std::vector<std::string> & vars,
                                                     const std::vector<std::string> & vals) :
    _feproblem(feproblem),
    _function_str(function_str),
    _vars(vars),
    _vals_input(vals)
{
  // Initialize (prepares Postprocessor values)
  initialize();

  // Create the libMesh::ParsedFunction
  _function_ptr = new ParsedFunction<Real>(_function_str, &_vars, &_vals);

  // Loop through the Postprocessor variables and point the libMesh::ParsedFunction to the PostprocessorValue
  for (unsigned int i = 0; i < _pp_index.size(); ++i)
    _addr.push_back(&_function_ptr->getVarAddress(_vars[_pp_index[i]]));
}

MooseParsedFunctionWrapper::~MooseParsedFunctionWrapper()
{
  delete _function_ptr;
}

template<>
Real
MooseParsedFunctionWrapper::evaluate(Real t, const Point & p)
{
  // Update the postprocessor / libMesh::ParsedFunction references for the desired function
  update();

  // Evalute the function that returns a scalar
  return (*_function_ptr)(p, t);
}

template<>
DenseVector<Real>
MooseParsedFunctionWrapper::evaluate(Real t, const Point & p)
{
  update();
  DenseVector<Real> output(LIBMESH_DIM);
  (*_function_ptr)(p, t, output);
  return output;
}

template<>
RealVectorValue
MooseParsedFunctionWrapper::evaluate(Real t, const Point & p)
{
  DenseVector<Real> output = evaluate<DenseVector<Real> >(t, p);

  return RealVectorValue(output(0)
#if LIBMESH_DIM > 1
                      , output(1)
#endif
#if LIBMESH_DIM > 2
                      , output(2)
#endif
    );
}

void
MooseParsedFunctionWrapper::initialize()
{
  // Loop through all the input values supplied by the users.
  for (unsigned int i=0; i < _vals_input.size(); ++i)
  {
    Real tmp; // desired type
    std::istringstream ss(_vals_input[i]); // istringstream object for conversion from std::string to Real

    // Case when a Postprocessor is found by the name given in the input values
    if (_feproblem.hasPostprocessor(_vals_input[i]))
    {
      // Store a pointer to the Postprocessor value
      _pp_vals.push_back(&_feproblem.getPostprocessorValue(_vals_input[i]));

      // Store the value for passing to the the libMesh::ParsedFunction
      _vals.push_back(_feproblem.getPostprocessorValue(_vals_input[i]));

      // Store the location of this variable
      _pp_index.push_back(i);
    }

    // Case when a Real is supplied, convert std::string to Real
    else
    {
      // Use istringstream to convert, if it fails produce an error, otherwise add the variable to the _vals variable
      if (!(ss >> tmp))
        mooseError("The input value '" << _vals_input[i] << "' was not understood, it must be a Real or a Postprocessor");
      else
        _vals.push_back(tmp);
    }
  }
}

void
MooseParsedFunctionWrapper::update()
{
  for (unsigned int i = 0; i < _pp_index.size(); ++i)
    (*_addr[i]) = (*_pp_vals[i]);
}
