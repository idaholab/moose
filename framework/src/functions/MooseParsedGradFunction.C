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
    _grad_function(new ParsedFunction<Real>(std::string("{") + getParam<std::string>("grad_x") + "}{" +
                                            getParam<std::string>("grad_y") + "}{" +
                                            getParam<std::string>("grad_z") + "}",
                                            &getParam<std::vector<std::string> >("vars"),
                                            &getParam<std::vector<Real> >("vals")))
{}

MooseParsedGradFunction::~MooseParsedGradFunction()
{
  delete _grad_function;
}


RealGradient
MooseParsedGradFunction::gradient(Real t, const Point & pt)
{
  DenseVector<Real> output(LIBMESH_DIM);

  (*_grad_function)(pt, t, output);

  return RealGradient(output(0)
#if LIBMESH_DIM > 1
                      , output(1)
#endif
#if LIBMESH_DIM > 2
                      , output(2)
#endif
    );
}
