/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

//  Fluid density assuming constant bulk modulus, but reducing via a cubic to zero at p=zero_point
//
#include "RichardsDensityConstBulkCut.h"

template <>
InputParameters
validParams<RichardsDensityConstBulkCut>()
{
  InputParameters params = validParams<RichardsDensity>();
  params.addRequiredParam<Real>("dens0", "Reference density of fluid.  Eg 1000");
  params.addRequiredParam<Real>("bulk_mod", "Bulk modulus of fluid.  Eg 2E9");
  params.addParam<Real>("cut_limit",
                        1E5,
                        "For porepressure > cut_limit, density = dens0*Exp(pressure/bulk).  For "
                        "porepressure < cut_limie, density = cubic*dens0*Exp(pressure/bulk), where "
                        "cubic=1 for pressure=cut_limit, and cubic=0 for pressure<=zero_point");
  params.addParam<Real>("zero_point",
                        0,
                        "For porepressure > cut_limit, density = dens0*Exp(pressure/bulk).  For "
                        "porepressure < cut_limie, density = cubic*dens0*Exp(pressure/bulk), where "
                        "cubic=1 for pressure=cut_limit, and cubic=0 for pressure<=zero_point");
  params.addClassDescription(
      "Fluid density assuming constant bulk modulus.  dens0 * Exp(pressure/bulk)");
  return params;
}

RichardsDensityConstBulkCut::RichardsDensityConstBulkCut(const InputParameters & parameters)
  : RichardsDensity(parameters),
    _dens0(getParam<Real>("dens0")),
    _bulk(getParam<Real>("bulk_mod")),
    _cut_limit(getParam<Real>("cut_limit")),
    _zero_point(getParam<Real>("zero_point")),
    _c3(std::pow(_cut_limit - _zero_point, 3))
{
  if (_zero_point >= _cut_limit)
    mooseError("RichardsDensityConstantbulkCut: zero_point must be less than cut_limit");
}

Real
RichardsDensityConstBulkCut::density(Real p) const
{
  if (p <= _zero_point)
    return 0;

  Real unmodified = _dens0 * std::exp(p / _bulk);
  if (p >= _cut_limit)
    return unmodified;

  Real modifier =
      (3 * _cut_limit - 2 * p - _zero_point) * (p - _zero_point) * (p - _zero_point) / _c3;
  return modifier * unmodified;
}

Real
RichardsDensityConstBulkCut::ddensity(Real p) const
{
  if (p <= _zero_point)
    return 0;

  Real unmodified = _dens0 * std::exp(p / _bulk);
  if (p >= _cut_limit)
    return unmodified / _bulk;

  Real modifier =
      (3 * _cut_limit - 2 * p - _zero_point) * (p - _zero_point) * (p - _zero_point) / _c3;
  Real dmodifier = 6 * (_cut_limit - p) * (p - _zero_point) / _c3;
  return (modifier / _bulk + dmodifier) * unmodified;
}

Real
RichardsDensityConstBulkCut::d2density(Real p) const
{
  if (p <= _zero_point)
    return 0;

  Real unmodified = _dens0 * std::exp(p / _bulk);
  if (p >= _cut_limit)
    return unmodified / _bulk / _bulk;

  Real modifier =
      (3 * _cut_limit - 2 * p - _zero_point) * (p - _zero_point) * (p - _zero_point) / _c3;
  Real dmodifier = 6 * (_cut_limit - p) * (p - _zero_point) / _c3;
  Real d2modifier = (6 * _cut_limit + 6 * _zero_point - 12 * p) / _c3;
  return (modifier / _bulk / _bulk + 2 * dmodifier / _bulk + d2modifier) * unmodified;
}
