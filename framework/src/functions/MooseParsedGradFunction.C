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

#include "MooseParsedGradFunction.h"

template<>
InputParameters validParams<MooseParsedGradFunction>()
{
  InputParameters params = validParams<MooseParsedFunction>();
  params.addParam<std::string>("grad_x", "0", "Partial with respect to x.");
  params.addParam<std::string>("grad_y", "0", "Partial with respect to y.");
  params.addParam<std::string>("grad_z", "0", "Partial with respect to z.");
  return params;
}

MooseParsedGradFunction::MooseParsedGradFunction(const std::string & name, InputParameters parameters) :
    MooseParsedFunction(name, parameters),
    _grad_value(std::string("{") + getParam<std::string>("grad_x") + "}{" +
                getParam<std::string>("grad_y") + "}{" +
                getParam<std::string>("grad_z") + "}")
{}

MooseParsedGradFunction::~MooseParsedGradFunction()
{
  delete _grad_function; // remove the libMesh::ParsedFunction for the gradient
}

RealGradient
MooseParsedGradFunction::gradient(Real t, const Point & pt)
{
  // Define the output vector
  DenseVector<Real> output(LIBMESH_DIM);

  // Evaluate the gradient libMesh::ParsedFunction
  (*_grad_function)(pt, t, output);

  // Return the gradient, sized correctly for the dimension of the problem
  return RealGradient(output(0)
#if LIBMESH_DIM > 1
                      , output(1)
#endif
#if LIBMESH_DIM > 2
                      , output(2)
#endif
    );
}

void
MooseParsedGradFunction::updatePostprocessorValues()
{
  // Loop through the variables that are Postprocessors and update the libMesh::ParsedFunction
  for (unsigned int i = 0; i < _pp_index.size(); ++i)
  {
    (*_var_addr[i]) = (*_pp_vals[i]);      // updates values in _function
    (*_grad_var_addr[i]) = (*_pp_vals[i]); //updates values in _grad_function
  }
}


void
MooseParsedGradFunction::initialSetup()
{
  // Call the initialSetup from the base class (MooseParsedFunction)
  MooseParsedFunction::initialSetup();

  // Create the libMesh::ParsedFunction for the gradient
  _grad_function = new ParsedFunction<Real>(_grad_value, &_vars, &_vals);

  // Store pointers to the variables of the libMesh::ParsedFunction
 for (unsigned int i = 0; i < _pp_index.size(); ++i)
   _grad_var_addr.push_back(&_grad_function->getVarAddress(_vars[_pp_index[i]]));

}
