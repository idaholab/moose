#include "LinearInterpolationMaterial.h"

template<>
InputParameters validParams<LinearInterpolationMaterial>()
{
  InputParameters params = validParams<Material>();
  params.addRequiredParam<std::string>("prop_name", "The property name that will contain the piecewise function");
  params.addRequiredParam<std::vector<Real> >("independent_vals", "The vector of indepedent values for building the piecewise function");
  params.addRequiredParam<std::vector<Real> >("dependent_vals", "The vector of depedent values for building the piecewise function");
  return params;
}


LinearInterpolationMaterial::LinearInterpolationMaterial(const std::string & name, InputParameters parameters) :
    Material(name, parameters),
    _piecewise_func(getParam<std::vector<Real> >("independent_vals"),
                    getParam<std::vector<Real> >("dependent_vals")),
    _property(declareProperty<Real>(getParam<std::string>("prop_name")))
{
  _piecewise_func.dumpSampleFile(getParam<std::string>("prop_name"),
                                 "X position",
                                 getParam<std::string>("prop_name"));
}

void
LinearInterpolationMaterial::computeQpProperties()
{
  // We are just going to sample with the current X coordinate
  _property[_qp] = _piecewise_func.sample(_coord[0]);
}
