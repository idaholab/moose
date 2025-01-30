#pragma once

#include "Multilinear6DInterpolationModel.h"

namespace neml2
{
class TabulatedDislocationDensity : public Multilinear6DInterpolationModel
{
public:
  TabulatedDislocationDensity(const OptionSet & options);

  static OptionSet expected_options();

protected:
  void set_value(bool, bool, bool) override;
  /// transform output data
  virtual Scalar transform(const Scalar & data) override;

private:
  /// Struct members from CompressTransform in lafleur struct
  //@{
  const Real _factor;
  const Real _compressor;
  const Real _original_min;
  //@}
};
} // namespace neml2
