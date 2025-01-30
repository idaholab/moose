#include "TabulatedDislocationDensity.h"
#include <fstream>
#include "neml2/misc/math.h"

namespace neml2
{
register_NEML2_object(TabulatedDislocationDensity);

OptionSet
TabulatedDislocationDensity::expected_options()
{
  auto options = Multilinear6DInterpolationModel::expected_options();
  // Model constants  fixme make these required
  options.set<Real>("factor") = 0.0;
  options.set<Real>("compressor") = 0.0;
  options.set<Real>("original_min") = 1.0;
  // fixme lynn do I want to use AD because dislocations densities don't need derivative
  options.set<bool>("_use_AD_first_derivative") = true;
  options.set<bool>("_use_AD_second_derivative") = true;
  return options;
}

TabulatedDislocationDensity::TabulatedDislocationDensity(const OptionSet & options)
  : Multilinear6DInterpolationModel(options),
    _factor(options.get<Real>("factor")),
    _compressor(options.get<Real>("compressor")),
    _original_min(options.get<Real>("original_min"))
{
}

void
TabulatedDislocationDensity::set_value(bool out, bool dout_din, bool d2out_din2)
{
  neml_assert_dbg(!dout_din || !d2out_din2,
                  "Only AD derivatives are currently supported for this model");
  if (out)
  {
    _output_rate = interpolate_and_transform();
  }
}

Scalar
TabulatedDislocationDensity::transform(const Scalar & data)
{
  auto d1 = math::pow(10.0, data) - 1.0 + _original_min;
  auto transformed_data = math::sign(d1) * math::pow(math::abs(d1), 1.0 / _compressor) / _factor;
  return transformed_data;
}
} // namespace neml2
