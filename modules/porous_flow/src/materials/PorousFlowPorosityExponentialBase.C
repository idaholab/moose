/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "PorousFlowPorosityExponentialBase.h"

template <>
InputParameters
validParams<PorousFlowPorosityExponentialBase>()
{
  InputParameters params = validParams<PorousFlowPorosityBase>();
  params.addParam<bool>("strain_at_nearest_qp",
                        false,
                        "When calculating nodal porosity that depends on strain, use the strain at "
                        "the nearest quadpoint.  This adds a small extra computational burden, and "
                        "is not necessary for simulations involving only linear lagrange elements. "
                        " If you set this to true, you will also want to set the same parameter to "
                        "true for related Kernels and Materials");
  params.addParam<bool>("ensure_positive",
                        true,
                        "Modify the usual exponential relationships that "
                        "governs porosity so that porosity is always "
                        "positive");
  params.addClassDescription("Base class Material for porosity that is computed via an exponential "
                             "relationship with coupled variables (strain, porepressure, "
                             "temperature)");
  return params;
}

PorousFlowPorosityExponentialBase::PorousFlowPorosityExponentialBase(
    const InputParameters & parameters)
  : PorousFlowPorosityBase(parameters),
    _strain_at_nearest_qp(getParam<bool>("strain_at_nearest_qp")),
    _ensure_positive(getParam<bool>("ensure_positive"))
{
}

Real
PorousFlowPorosityExponentialBase::atNegInfinityQp() const
{
  return 1.0;
}

Real
PorousFlowPorosityExponentialBase::atZeroQp() const
{
  return 1.0;
}

Real
PorousFlowPorosityExponentialBase::decayQp() const
{
  return 0.0;
}

Real
PorousFlowPorosityExponentialBase::ddecayQp_dvar(unsigned /*pvar*/) const
{
  return 0.0;
}

RealGradient
PorousFlowPorosityExponentialBase::ddecayQp_dgradvar(unsigned /*pvar*/) const
{
  return RealGradient();
}

void
PorousFlowPorosityExponentialBase::initQpStatefulProperties()
{
  _porosity[_qp] = atZeroQp();
}

void
PorousFlowPorosityExponentialBase::computeQpProperties()
{
  const Real a = atNegInfinityQp();
  const Real b = atZeroQp();
  mooseAssert(a > b, "PorousFlowPorosityExponentialBase a must be larger than b");
  const Real decay = decayQp();

  Real deriv = 0.0; // = d(porosity)/d(decay)
  if (decay <= 0.0 || !_ensure_positive)
  {
    _porosity[_qp] = a + (b - a) * std::exp(decay);
    deriv = _porosity[_qp] - a;
  }
  else
  {
    const Real c = std::log(a / (a - b));
    const Real expx = std::exp(-decay / c);
    // note that at decay = 0, we have expx = 1, so porosity = a + b - a = b
    // and at decay = infinity, expx = 0, so porosity = a + (b - a) * a / (a - b) = 0
    _porosity[_qp] = a + (b - a) * std::exp(c * (1.0 - expx));
    deriv = (_porosity[_qp] - a) * expx;
  }

  _dporosity_dvar[_qp].resize(_num_var);
  _dporosity_dgradvar[_qp].resize(_num_var);
  for (unsigned int v = 0; v < _num_var; ++v)
  {
    _dporosity_dvar[_qp][v] = ddecayQp_dvar(v) * deriv;
    _dporosity_dgradvar[_qp][v] = ddecayQp_dgradvar(v) * deriv;
  }
}
