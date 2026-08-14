//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PorousFlowPorosityExponentialBase.h"

#include <limits>

template <bool is_ad>
InputParameters
PorousFlowPorosityExponentialBaseTempl<is_ad>::validParams()
{
  InputParameters params = PorousFlowPorosityBaseTempl<is_ad>::validParams();
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
  params.addParam<Real>(
      "porosity_min",
      std::numeric_limits<Real>::lowest(),
      "Minimum allowed value of the porosity: if the computed porosity is less than this value, "
      "porosity is set to this value instead.  By default no floor is imposed.  The "
      "ensure_positive "
      "transform only acts for decay > 0, so chemistry-driven (precipitation) porosity is "
      "otherwise "
      "unbounded below and can become negative once pore space is filled by mineral; set this to a "
      "small positive value in that case.");
  params.addParam<Real>("zero_modifier",
                        1E-3,
                        "If the porosity_min floor is active, the porosity derivatives are set to "
                        "zero_modifier times their unfloored values (rather than exactly zero) to "
                        "hint to the Newton-Krylov nonlinear solver that porosity "
                        "is not strictly constant, which aids convergence");
  params.addParamNamesToGroup("zero_modifier", "Advanced");
  params.addClassDescription("Base class Material for porosity that is computed via an exponential "
                             "relationship with coupled variables (strain, porepressure, "
                             "temperature, chemistry)");
  return params;
}

template <bool is_ad>
PorousFlowPorosityExponentialBaseTempl<is_ad>::PorousFlowPorosityExponentialBaseTempl(
    const InputParameters & parameters)
  : PorousFlowPorosityBaseTempl<is_ad>(parameters),
    _strain_at_nearest_qp(this->template getParam<bool>("strain_at_nearest_qp")),
    _ensure_positive(this->template getParam<bool>("ensure_positive")),
    _porosity_min(this->template getParam<Real>("porosity_min")),
    _zero_modifier(this->template getParam<Real>("zero_modifier"))
{
}

template <bool is_ad>
void
PorousFlowPorosityExponentialBaseTempl<is_ad>::initQpStatefulProperties()
{
  // Unqualified calls so ADL finds the MetaPhysicL overloads for the AD (DualNumber) instantiation
  using std::exp;
  using std::log;

  const GenericReal<is_ad> a = atNegInfinityQp();
  const GenericReal<is_ad> b = atZeroQp();
  mooseAssert(a > b, "PorousFlowPorosityExponentialBase a must be larger than b");
  const GenericReal<is_ad> decay = decayQp();

  if (decay <= 0.0 || !_ensure_positive)
    _porosity[_qp] = a + (b - a) * exp(decay);
  else
  {
    const GenericReal<is_ad> c = log(a / (a - b));
    const GenericReal<is_ad> expx = exp(-decay / c);
    _porosity[_qp] = a + (b - a) * exp(c * (1.0 - expx));
  }

  if (_porosity[_qp] < _porosity_min)
    _porosity[_qp] = _porosity_min;
}

template <bool is_ad>
void
PorousFlowPorosityExponentialBaseTempl<is_ad>::computeQpProperties()
{
  // Unqualified calls so ADL finds the MetaPhysicL overloads for the AD (DualNumber) instantiation
  using std::exp;
  using std::log;
  using std::pow;

  const GenericReal<is_ad> a = atNegInfinityQp();
  const GenericReal<is_ad> b = atZeroQp();
  const GenericReal<is_ad> decay = decayQp();
  GenericReal<is_ad> exp_term = 1.0; // set appropriately below

  if (decay <= 0.0 || !_ensure_positive)
  {
    exp_term = exp(decay);
    _porosity[_qp] = a + (b - a) * exp_term;
  }
  else
  {
    const GenericReal<is_ad> c = log(a / (a - b));
    // note that at decay = 0, we have expx = 1, so porosity = a + b - a = b
    // and at decay = infinity, expx = 0, so porosity = a + (b - a) * a / (a - b) = 0
    const GenericReal<is_ad> expx = exp(-decay / c);
    exp_term = exp(c * (1.0 - expx));
    _porosity[_qp] = a + (b - a) * exp_term;
  }

  if constexpr (!is_ad)
  {
    // The AD path carries the derivatives with respect to the PorousFlow variables inside the AD
    // porosity value, so only the non-AD path builds these derivative material properties.
    Real deriv = 0.0; // = d(porosity)/d(decay)
    if (decay <= 0.0 || !_ensure_positive)
      deriv = _porosity[_qp] - a;
    else
    {
      const Real c = log(a / (a - b));
      const Real expx = exp(-decay / c);
      deriv = (_porosity[_qp] - a) * expx;
    }

    (*_dporosity_dvar)[_qp].resize(_num_var);
    (*_dporosity_dgradvar)[_qp].resize(_num_var);
    for (unsigned int v = 0; v < _num_var; ++v)
    {
      (*_dporosity_dvar)[_qp][v] = ddecayQp_dvar(v) * deriv;
      (*_dporosity_dgradvar)[_qp][v] = ddecayQp_dgradvar(v) * deriv;

      const Real da = datNegInfinityQp(v);
      const Real db = datZeroQp(v);
      (*_dporosity_dvar)[_qp][v] += da * (1 - exp_term) + db * exp_term;

      if (!(decay <= 0.0 || !_ensure_positive))
      {
        const Real c = log(a / (a - b));
        const Real expx = exp(-decay / c);
        const Real dc = (a - b) * (da * b / a - db) / pow(a, 2);
        (*_dporosity_dvar)[_qp][v] += (b - a) * exp_term * dc * (1 - expx - expx / c);
      }
    }
  }

  // Apply the porosity floor last, after the unfloored derivatives above have been
  // formed.  When floored, soften the derivatives with _zero_modifier so the Newton
  // process still sees porosity as weakly varying (cf. PorousFlowPorosityLinear).
  if (_porosity[_qp] < _porosity_min)
  {
    if constexpr (!is_ad)
    {
      _porosity[_qp] = _porosity_min;
      for (unsigned int v = 0; v < _num_var; ++v)
      {
        (*_dporosity_dvar)[_qp][v] *= _zero_modifier;
        (*_dporosity_dgradvar)[_qp][v] *= _zero_modifier;
      }
    }
    else
    {
      // The AD path carries its derivatives inside the value and has no derivative material
      // properties (they are nullptr), so the value and the seeds are softened separately here.
      // Assigning _porosity_min to the ADReal would instead discard the derivatives entirely.
      _porosity[_qp].value() = _porosity_min;
      _porosity[_qp].derivatives() *= _zero_modifier;
    }
  }
}

template class PorousFlowPorosityExponentialBaseTempl<false>;
template class PorousFlowPorosityExponentialBaseTempl<true>;
