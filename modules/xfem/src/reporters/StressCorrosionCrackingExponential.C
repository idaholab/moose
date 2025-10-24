//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "StressCorrosionCrackingExponential.h"
#include "VectorPostprocessorInterface.h"

registerMooseObject("XFEMApp", StressCorrosionCrackingExponential);

InputParameters
StressCorrosionCrackingExponential::validParams()
{
  InputParameters params = CrackGrowthReporterBase::validParams();
  params.addClassDescription(
      "This reporter computes the crack growth increment at all active crack front points "
      "in the CrackMeshCut3DUserObject for stress corrosion cracking fit to an exponential "
      "function. Crack growth rates computed by this reporter are stored in the same order as in "
      "the fracture integral VectorPostprocessors.");

  params.addRequiredRangeCheckedParam<Real>("k_low",
                                            "k_low>0",
                                            "Value of K_I below which the crack growth rate is "
                                            "constant and equal to the mid growth rate function "
                                            "evaluated with a K_I=k_low.");
  params.addRequiredRangeCheckedParam<Real>("k_high",
                                            "k_high>0",
                                            "Value of K_I above which the crack growth rate is "
                                            "constant and equal to the mid growth rate function "
                                            "evaluated with a K_I=k_high.");
  params.addRequiredParam<Real>("growth_rate_mid_multiplier",
                                "Growth rate multiplier when K_I is between k_low and k_high");
  params.addRequiredParam<Real>("growth_rate_mid_exp_factor", "Growth rate exponential factor");

  params.addParam<ReporterValueName>(
      "growth_increment_name",
      "growth_increment",
      "ReporterValueName for storing computed growth increments for the crack front points.");
  params.addParam<ReporterValueName>(
      "time_to_max_growth_increment_name",
      "max_growth_timestep",
      "ReporterValueName for storing computed timestep to reach max_growth_increment.");
  return params;
}

StressCorrosionCrackingExponential::StressCorrosionCrackingExponential(
    const InputParameters & parameters)
  : CrackGrowthReporterBase(parameters),
    _k_low(getParam<Real>("k_low")),
    _k_high(getParam<Real>("k_high")),
    _growth_rate_mid_multiplier(getParam<Real>("growth_rate_mid_multiplier")),
    _growth_rate_mid_exp_factor(getParam<Real>("growth_rate_mid_exp_factor")),

    _time_increment(declareValueByName<Real>(
        getParam<ReporterValueName>("time_to_max_growth_increment_name"), REPORTER_MODE_ROOT)),
    _growth_increment(declareValueByName<std::vector<Real>>(
        getParam<ReporterValueName>("growth_increment_name"), REPORTER_MODE_ROOT))
{
  if (_k_low > _k_high)
    paramError("k_high", "k_high must be larger than k_low");
}

void
StressCorrosionCrackingExponential::computeGrowth(std::vector<int> & index)
{
  _growth_increment.resize(_ki_x.size(), 0.0);
  std::vector<Real> growth_rate(_ki_x.size(), 0.0);

  for (std::size_t i = 0; i < _ki_vpp.size(); ++i)
  {
    if (index[i] != -1)
    {
      Real ki = _ki_vpp[i];
      if (ki < _k_low)
        ki = _k_low;
      else if (ki > _k_high)
        ki = _k_high;

      growth_rate[i] = _growth_rate_mid_multiplier * std::pow(ki, _growth_rate_mid_exp_factor);
    }
  }

  Real max_growth_rate = *std::max_element(growth_rate.begin(), growth_rate.end());
  if (max_growth_rate <= 0)
    mooseError("Negative max growth rate encountered on crack front. ");

  _time_increment = _max_growth_increment / max_growth_rate;

  for (std::size_t i = 0; i < _ki_vpp.size(); ++i)
  {
    if (index[i] != -1)
      _growth_increment[i] = growth_rate[i] * _time_increment;
  }
}
