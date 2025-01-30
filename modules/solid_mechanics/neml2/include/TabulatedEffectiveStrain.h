#pragma once

#include "Multilinear6DInterpolationModel.h"

namespace neml2
{
class TabulatedEffectiveStrain : public Multilinear6DInterpolationModel
{
public:
  TabulatedEffectiveStrain(const OptionSet & options);

  static OptionSet expected_options();

protected:
  void set_value(bool, bool, bool) override;

  /// transform output data
  virtual Scalar transform(const Scalar & data) override;

private:
  /// Struct members from Log10Transform struct in laromance
  //@{
  const Real _factor;
  const Real _lowerbound;
  const Real _upperbound;
  const Real _logmin;
  const Real _logmax;
  //@}
};
} // namespace neml2
