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

ParsedGradFunction::ParsedGradFunction(const std::string & name, InputParameters parameters) :
    ParsedFunction(name, parameters)
{
  std::vector<std::string> vars = parameters.get<std::vector<std::string> >("vars");
  std::vector<Real>        vals = parameters.get<std::vector<Real> >("vals");

  initializeParser(_parserx, parameters.get<std::string>("grad_x"), vars, vals);
  initializeParser(_parsery, parameters.get<std::string>("grad_y"), vars, vals);
  initializeParser(_parserz, parameters.get<std::string>("grad_z"), vars, vals);
}

RealGradient
ParsedGradFunction::gradient(Real t, const Point & pt)
{
  _vals[0] = t;
  _vals[1] = pt(0);
  _vals[2] = pt(1);
  _vals[3] = pt(2);

  return RealGradient(_parserx.Eval(&(_vals[0])),
                      _parsery.Eval(&(_vals[0])),
                      _parserz.Eval(&(_vals[0])));
}
