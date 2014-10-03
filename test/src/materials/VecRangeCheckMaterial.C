#include "VecRangeCheckMaterial.h"

template<>
InputParameters validParams<VecRangeCheckMaterial>()
{
  InputParameters params = validParams<Material>();
  params.addRequiredRangeCheckedParam<std::vector<Real> >("rv3", "rv3_size = 3", "Real vector of length 3");
  params.addRequiredRangeCheckedParam<std::vector<Real> >("iv3", "iv3_size = 3", "Int vector of length 3");
  params.addRequiredRangeCheckedParam<std::vector<Real> >("rvp", "rvp > 0", "Real vector of all positive values");
  params.addRequiredRangeCheckedParam<std::vector<Real> >("ivg", "ivg_0 > ivg_1", "Int vector where component 0 is bigger than component 1");
  params.addRequiredRangeCheckedParam<std::vector<Real> >("rvl", "rvl_10 > 0", "Testing if component 10 is positive (usually we should have a size check here as well)");
  return params;
}

VecRangeCheckMaterial::VecRangeCheckMaterial(const std::string & name, InputParameters parameters) :
    Material(name, parameters)
{
}
