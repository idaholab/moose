//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PorousFlowPorosityExponentialBase.h"

InputParameters
PorousFlowPorosityExponentialBase::validParams()
{
  InputParameters params = PorousFlowPorosityBase::validParams();
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
                             "temperature, chemistry)");
  return params;
}

PorousFlowPorosityExponentialBase::PorousFlowPorosityExponentialBase(
    const InputParameters & parameters)
  : PorousFlowPorosityBase(parameters),
    _strain_at_nearest_qp(getParam<bool>("strain_at_nearest_qp")),
    _ensure_positive(getParam<bool>("ensure_positive"))
{
}

void
PorousFlowPorosityExponentialBase::initQpStatefulProperties()
{
  const Real a = atNegInfinityQp();
  const Real b = atZeroQp();
  mooseAssert(a > b, "PorousFlowPorosityExponentialBase a must be larger than b");
  const Real decay = decayQp();

  if (decay <= 0.0 || !_ensure_positive)
    _porosity[_qp] = a + (b - a) * std::exp(decay);
  else
  {
    const Real c = std::log(a / (a - b));
    const Real expx = std::exp(-decay / c);
    _porosity[_qp] = a + (b - a) * std::exp(c * (1.0 - expx));
  }
}

void
PorousFlowPorosityExponentialBase::computeQpProperties()
{
  const Real a = atNegInfinityQp();
  const Real b = atZeroQp();
  const Real decay = decayQp();
  Real exp_term = 1.0; // set appropriately below

  Real deriv = 0.0; // = d(porosity)/d(decay)
  if (decay <= 0.0 || !_ensure_positive)
  {
    exp_term = std::exp(decay);
    _porosity[_qp] = a + (b - a) * exp_term;
    deriv = _porosity[_qp] - a;
  }
  else
  {
    const Real c = std::log(a / (a - b));
    const Real expx = std::exp(-decay / c);
    // note that at decay = 0, we have expx = 1, so porosity = a + b - a = b
    // and at decay = infinity, expx = 0, so porosity = a + (b - a) * a / (a - b) = 0
    exp_term = std::exp(c * (1.0 - expx));
    _porosity[_qp] = a + (b - a) * exp_term;
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
      const Real c = std::log(a / (a - b));
      const Real expx = std::exp(-decay / c);
      const Real dc = (a - b) * (da * b / a - db) / std::pow(a, 2);
      (*_dporosity_dvar)[_qp][v] += (b - a) * exp_term * dc * (1 - expx - expx / c);
    }
  }
}
