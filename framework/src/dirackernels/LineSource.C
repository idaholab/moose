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

#include "LineSource.h"

// libmesh includes
#include "libmesh/quadrature.h"

template <>
InputParameters
validParams<LineSource>()
{
  InputParameters params = validParams<DiracKernel>();
  params += validParams<FunctionParserUtils>();
  params.addRequiredParam<Real>("tmin", "Minimum parameter t along curve");
  params.addRequiredParam<Real>("tmax", "Maximum parameter t along curve");
  params.addRequiredParam<std::string>("x",
                                       "Function expression encoding the x coordinate. Use parameter s");
  params.addRequiredParam<std::string>("y",
                                       "Function expression encoding the y coordinate. Use parameter s");
  params.addRequiredParam<std::string>("z",
                                       "Function expression encoding the z coordinate. Use parameter s");
  params.addRequiredParam<std::string>("length",
                                       "Function expression encoding the length along the curve. Use parameter s");
  params.addRequiredParam<std::string>("strength",
                                       "Function expression encoding the strength along the curve. Use parameter s");
  params.addParam<std::vector<std::string>>(
      "constant_names", "Vector of constants used in the parsed function (use this for kB etc.)");
  params.addParam<std::vector<std::string>>(
      "constant_expressions",
      "Vector of values for the constants in constant_names (can be an FParser expression)");
  return params;
}

LineSource::LineSource(const InputParameters & parameters)
  : DiracKernel(parameters),
    FunctionParserUtils(parameters),
    _tmin(getParam<Real>("tmin")),
    _tmax(getParam<Real>("tmax")),
    _eps(1e-12)
{
  _curve.resize(3);
  setupParsedFunctionObject(parameters.get<std::string>("x"), _curve[0]);
  setupParsedFunctionObject(parameters.get<std::string>("y"), _curve[1]);
  setupParsedFunctionObject(parameters.get<std::string>("z"), _curve[2]);
  setupParsedFunctionObject(parameters.get<std::string>("length"), _length);
  setupParsedFunctionObject(parameters.get<std::string>("strength"), _strength);
  _func_params.resize(1);

  // get a Gaussian quadrature from libmesh
  std::shared_ptr<QBase> _quadrature = QBase::build(QGAUSS, 1, SIXTH);
  _quadrature->get_points();
  _quadrature->get_weights();

  // get the total length of the curve
  _func_params[0] = _tmin;
  Real lmin = evaluate(_length);
  _func_params[0] = _tmax;
  Real lmax = evaluate(_length);
  Real total_length = lmax - lmin;

  // distribute the Gaussian integration points and assign weights
  std::vector<Real> gauss_l;
  std::vector<Real> gauss_w;
  for (unsigned int j = 0; j < _quadrature->get_points().size(); ++j)
  {
    // get the point and find proper distance on curve; distribute proportionally
    // according to the length measured along the curve _and_ not according to t
    Real l = (_quadrature->get_points()[j](0) + 1) / 2 * total_length + lmin;
    gauss_l.push_back(l);

    // adjust the weight as well; the 0.5 comes from the sum of the of the weights
    // of Gaussian quadrature == 2
    Real w = total_length * 0.5 * _quadrature->get_weights()[j];
    gauss_w.push_back(w);
  }

  // now find the correct t values corresponding to _gauss_l using bisection
  std::vector<Real> gauss_t;

  for (unsigned int j = 0; j < _quadrature->get_points().size(); ++j)
  {
    unsigned int iter = 0;
    Real tl = _tmin;
    Real to = _tmax;
    Real l;
    Real t;

    do
    {
      ++iter;
      if (iter > 10000)
        mooseError("Bisection does not converge");

      t = 0.5 * (tl + to);
      _func_params[0] = t;
      l = evaluate(_length);
      _func_params[0] = tl;
      Real ll = evaluate(_length);
      if ((gauss_l[j] - l) * (gauss_l[j] - ll) > 0)
        tl = t;
      else
        to = t;
    }
    while (std::abs(gauss_l[j] - l) > _eps);
    gauss_t.push_back(t);
  }

  // create the array of physical points
  for (unsigned int j = 0; j < gauss_w.size(); ++j)
  {
    _func_params[0] = gauss_t[j];
    Point pt = Point(evaluate(_curve[0]), evaluate(_curve[1]), evaluate(_curve[2]));
    _points.push_back(pt);
    _point_strength[pt] = gauss_w[j] * evaluate(_strength);
  }
}

void
LineSource::setupParsedFunctionObject(const std::string & function, ADFunctionPtr & func_F)
{
  // base function object
  func_F = ADFunctionPtr(new ADFunction());

  // set FParser internal feature flags
  setParserFeatureFlags(func_F);

  // add the constant expressions
  addFParserConstants(func_F,
                      getParam<std::vector<std::string>>("constant_names"),
                      getParam<std::vector<std::string>>("constant_expressions"));

  // parse function
  if (func_F->Parse(function, "s") >= 0)
    mooseError("Invalid function\n",
               function,
               "\nin LineSource ",
               name(),
               ".\n",
               func_F->ErrorMsg());
}

void
LineSource::addPoints()
{
  for (auto & p : _points)
    addPoint(p);
}

Real
LineSource::computeQpResidual()
{
  Real strength = 0;
  if (_point_strength.find(_current_point) != _point_strength.end())
    strength = _point_strength.find(_current_point)->second;
  return -_test[_i][_qp] * strength;
}
