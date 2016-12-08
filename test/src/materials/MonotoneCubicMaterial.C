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
#include "MonotoneCubicMaterial.h"

template<>
InputParameters validParams<MonotoneCubicMaterial>()
{
  InputParameters params = validParams<Material>();
  params.addRequiredParam<std::string>("prop_name", "The property name that will contain the piecewise function");
  params.addRequiredParam<std::vector<Real> >("independent_vals", "The vector of indepedent values for building the piecewise function");
  params.addRequiredParam<std::vector<Real> >("dependent_vals", "The vector of depedent values for building the piecewise function");
  params.addParam<unsigned int>("resolution", 100, "The size of the interpolated vector.");
  return params;
}

MonotoneCubicMaterial::MonotoneCubicMaterial(const InputParameters & parameters) :
    Material(parameters),
    _property(declareProperty<Real>(getParam<std::string>("prop_name")))
{
  std::vector<Real> x = getParam<std::vector<Real> >("independent_vals");
  std::vector<Real> y = getParam<std::vector<Real> >("dependent_vals");
  _monotone_interp.setData(x, y);

  unsigned int n = getParam<unsigned int>("resolution") + 1;
  std::vector<Real> xnew(n);
  Real step_size = (x.back() - x[0]) / (n - 1);
  for (unsigned int i = 0; i < n; ++i)
    xnew[i] = x[0] + i * step_size;

  _monotone_interp.dumpCSV(getParam<std::string>("prop_name") + ".csv", xnew);
}

void
MonotoneCubicMaterial::computeQpProperties()
{
  _property[_qp] = _monotone_interp.sample(_q_point[_qp](0));
}
