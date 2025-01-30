#include "TabulatedEffectiveStrain.h"
#include <fstream>
#include "neml2/misc/math.h"

namespace neml2
{
register_NEML2_object(TabulatedEffectiveStrain);

OptionSet
TabulatedEffectiveStrain::expected_options()
{
  auto options = Multilinear6DInterpolationModel::expected_options();
  // Model constants
  options.set<Real>("factor");
  options.set<Real>("lowerbound");
  options.set<Real>("upperbound");
  options.set<Real>("logmin");
  options.set<Real>("logmax");
  // fixme lynn do I want to use AD
  options.set<bool>("_use_AD_first_derivative") = true;
  options.set<bool>("_use_AD_second_derivative") = true;
  return options;
}

TabulatedEffectiveStrain::TabulatedEffectiveStrain(const OptionSet & options)
  : Multilinear6DInterpolationModel(options),
    _factor(options.get<Real>("factor")),
    _lowerbound(options.get<Real>("lowerbound")),
    _upperbound(options.get<Real>("upperbound")),
    _logmin(options.get<Real>("logmin")),
    _logmax(options.get<Real>("logmax"))
{
}

void
TabulatedEffectiveStrain::set_value(bool out, bool dout_din, bool d2out_din2)
{
  neml_assert_dbg(!dout_din || !d2out_din2,
                  "Only AD derivatives are currently supported for this model");
  if (out)
  {
    _output_rate = interpolate_and_transform();
  }
  // fixme lynn this will work best if transform and interpolation are called seperatly
  // if (dout_din)
  // {
  //  put untransformed _output_rate into d_transform.  Multiply that by derivative of interpolation
  //   _output_rate.d(_s) = d_transform(compute_interpolation)*d_interpolation;
  // }
}

Scalar
TabulatedEffectiveStrain::transform(const Scalar & data)
{
  Real range = _upperbound - _lowerbound;
  auto transformed_data =
      (math::pow(10.0, ((data - _lowerbound) * (_logmax - _logmin) / range) + _logmin) - _factor);
  return transformed_data;
}

// need derivative of transform too
//  if (derivative)
//  {
//    x = std::pow(10, ((logmax - logmin) * (x - lowerbound) / range + logmin)) *
//        (std::log(10) * (logmax - logmin) / range);
//  }

} // namespace neml2
