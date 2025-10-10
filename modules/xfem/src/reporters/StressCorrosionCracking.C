//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "StressCorrosionCracking.h"
#include "VectorPostprocessorInterface.h"

registerMooseObject("XFEMApp", StressCorrosionCracking);

InputParameters
StressCorrosionCracking::validParams()
{
  InputParameters params = CrackGrowthReporterBase::validParams();
  params.addClassDescription(
      "This reporter computes the crack extension size at all active crack front points "
      "in the CrackMeshCut3DUserObject.  This reporter is in the same order as "
      "ki_vectorpostprocessor.");

  params.addRequiredParam<Real>(
      "k_low", "K1 integral below this value has constant growth rate of growth_rate_low");
  params.addRequiredParam<Real>("growth_rate_low", "growth rate when K1 is below k_low");
  params.addRequiredParam<Real>(
      "k_high", "K1 integral above this value has constant growth rate of growth_rate_high");
  params.addRequiredParam<Real>("growth_rate_high", "growth rate when K1 is above k_high");
  params.addRequiredParam<Real>("growth_rate_mid_multiplier",
                                "Growth rate multiplier when K1 is between k_low and k_high");
  params.addRequiredParam<Real>(
      "growth_rate_mid_exp_factor",
      "Growth rate exponential factor when K1 is between k_low and k_high");

  params.addParam<ReporterValueName>(
      "growth_increment_name",
      "growth_increment",
      "Reporter name containing growth increments for the crack front points.");
  params.addParam<ReporterValueName>(
      "time_to_max_growth_size_name",
      "max_growth_timestep",
      "Reporter name containing the timestep to reach max_growth_size.");
  return params;
}

StressCorrosionCracking::StressCorrosionCracking(const InputParameters & parameters)
  : CrackGrowthReporterBase(parameters),
    _k_low(getParam<Real>("k_low")),
    _growth_rate_low(getParam<Real>("growth_rate_low")),
    _k_high(getParam<Real>("k_high")),
    _growth_rate_high(getParam<Real>("growth_rate_high")),
    _growth_rate_mid_multiplier(getParam<Real>("growth_rate_mid_multiplier")),
    _growth_rate_mid_exp_factor(getParam<Real>("growth_rate_mid_exp_factor")),

    _corrosion_time_step(declareValueByName<Real>(
        getParam<ReporterValueName>("time_to_max_growth_size_name"), REPORTER_MODE_ROOT)),
    _growth_increment(declareValueByName<std::vector<Real>>(
        getParam<ReporterValueName>("growth_increment_name"), REPORTER_MODE_ROOT))
{
}

void
StressCorrosionCracking::compute_growth(std::vector<int> & index)
{
  _growth_increment.resize(_ki_x.size(), 0.0);
  std::vector<Real> growth_rate(_ki_x.size(), 0.0);
  for (std::size_t i = 0; i < _ki_vpp.size(); ++i)
  {
    if (index[i] != -1)
    {
      if (_ki_vpp[i] < _k_low)
        growth_rate[i] = _growth_rate_low;
      else if ((_ki_vpp[i] >= _k_low) && (_ki_vpp[i] < _k_high))
        growth_rate[i] =
            _growth_rate_mid_multiplier * std::pow(_ki_vpp[i], _growth_rate_mid_exp_factor);
      else
        growth_rate[i] = _growth_rate_high;
    }
  }

  Real max_growth_rate = *std::max_element(growth_rate.begin(), growth_rate.end());
  _corrosion_time_step = _max_growth_size / max_growth_rate;

  for (std::size_t i = 0; i < _ki_vpp.size(); ++i)
  {
    if (index[i] != -1)
      _growth_increment[i] = growth_rate[i] * _corrosion_time_step;
  }
}
