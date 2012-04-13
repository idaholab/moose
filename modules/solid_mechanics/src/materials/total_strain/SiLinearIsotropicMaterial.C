#include "SiLinearIsotropicMaterial.h"

template<>
InputParameters validParams<SiLinearIsotropicMaterial>()
{
  return validParams<LinearIsotropicMaterial>();
}

SiLinearIsotropicMaterial::SiLinearIsotropicMaterial(const std::string & name,
                                                     InputParameters parameters) :
    LinearIsotropicMaterial(name, parameters)
{}

Real
SiLinearIsotropicMaterial::computeAlpha()
{
  if (_has_temp)
  {
    return 3.776e-6 * std::exp(0.0001257 * _temp[_qp]) -
      7.627e-6 * std::exp(-0.005766 * _temp[_qp]);
  }
  else
    return _alpha;
}
