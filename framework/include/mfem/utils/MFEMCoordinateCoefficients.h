#ifdef MOOSE_MFEM_ENABLED

#pragma once

#include "MooseTypes.h"
#include "mfem.hpp"

#include <memory>
#include <string>

namespace Moose::MFEM
{

class MFEMCoordinateCoefficients
{
public:
  enum class CoordinateSystem
  {
    Cartesian,
    Cylindrical
  };

  MFEMCoordinateCoefficients(CoordinateSystem coord, Real inv_r_eps);

  /// Build the coordinate-dependent coefficients for the selected system.
  void build();

  /// Direct getters
  const mfem::Coefficient * getRadialCoefficient() const { return _r_coeff.get(); }
  const mfem::Coefficient * getInverseRadialCoefficient() const { return _inv_r_coeff.get(); }
  const mfem::Coefficient * getTwoPiRCoefficient() const { return _two_pi_r_coeff.get(); }
  const mfem::Coefficient * getMeasureWeightCoefficient() const { return _measure_weight.get(); }

  /// Name-based lookup for materials / coefficient manager
  const mfem::Coefficient * getBuiltinCoefficient(const std::string & name) const;

private:
  CoordinateSystem _coord;
  Real _inv_r_eps;

  std::unique_ptr<mfem::Coefficient> _r_coeff;
  std::unique_ptr<mfem::Coefficient> _inv_r_coeff;
  std::unique_ptr<mfem::Coefficient> _two_pi_r_coeff;
  std::unique_ptr<mfem::Coefficient> _measure_weight;
};

} // namespace Moose::MFEM

#endif