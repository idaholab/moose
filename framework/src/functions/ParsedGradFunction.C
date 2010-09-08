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

#include "ParsedGradFunction.h"

template<>
InputParameters validParams<ParsedGradFunction>()
{
  InputParameters params = validParams<ParsedFunction>();
  params.addParam<std::string>("grad_x", "0", "Partial with respect to x.");
  params.addParam<std::string>("grad_y", "0", "Partial with respect to y.");
  params.addParam<std::string>("grad_z", "0", "Partial with respect to z.");
  return params;
}

ParsedGradFunction::ParsedGradFunction(const std::string & name, MooseSystem & moose_system, InputParameters parameters):
  ParsedFunction(name, moose_system, parameters)
{
  std::vector<std::string> vars = parameters.get<std::vector<std::string> >("vars");
  std::vector<Real>        vals = parameters.get<std::vector<Real> >("vals");

  initializeParser(_parserx, parameters.get<std::string>("grad_x"), vars, vals);
  initializeParser(_parsery, parameters.get<std::string>("grad_y"), vars, vals);
  initializeParser(_parserz, parameters.get<std::string>("grad_z"), vars, vals);
}

RealGradient
ParsedGradFunction::gradient(Real t, Real x, Real y, Real z)
{
  _vals[0] = t;
  _vals[1] = x;
  _vals[2] = y;
  _vals[3] = z;

  return RealGradient(_parserx.Eval(&(_vals[0])),
                      _parsery.Eval(&(_vals[0])),
                      _parserz.Eval(&(_vals[0])));
}
