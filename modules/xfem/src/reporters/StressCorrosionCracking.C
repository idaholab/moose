//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "StressCorrosionCracking.h"
#include "CrackMeshCut3DUserObject.h"
#include "VectorPostprocessorInterface.h"
#include "MathUtils.h"

registerMooseObject("XFEMApp", StressCorrosionCracking);

InputParameters
StressCorrosionCracking::validParams()
{
  InputParameters params = GeneralReporter::validParams();
  params.addClassDescription(
      "This reporter computes the crack extension size at all active crack front points "
      "in the CrackMeshCut3DUserObject.  This reporter has been sorted by the activeBoundaryNodes "
      "from the CrackMeshCut3DUserObject");
  params.addRequiredParam<UserObjectName>("crackMeshCut3DUserObject_name",
                                          "The CrackMeshCut3DUserObject user object name");
  params.addRequiredParam<Real>(
      "max_growth_size",
      "the max growth size at the crack front in each increment of a fatigue simulation");

  params.addRequiredParam<Real>(
      "k_low", "K1 integral below this value has constant growth rate of growth_rate_low");
  params.addRequiredParam<Real>(
      "growth_rate_low",
      "growth rate when K1 is below k_low");
  params.addRequiredParam<Real>(
      "k_high", "K1 integral above this value has constant growth rate of growth_rate_high");
  params.addRequiredParam<Real>(
      "growth_rate_high",
      "growth rate when K1 is above k_high");
  params.addRequiredParam<Real>(
      "growth_rate_mid_multiplier",
      "Growth rate multiplier when K1 is between k_low and k_high");
  params.addRequiredParam<Real>(
      "growth_rate_mid_exp_factor",
      "Growth rate exponential factor when K1 is between k_low and k_high");

  params.addParam<VectorPostprocessorName>(
      "ki_vectorpostprocessor", "II_KI_1", "The name of the vectorpostprocessor that contains KI");
  params.addParam<ReporterValueName>(
      "growth_increment_name",
      "growth_increment",
      "Reporter name containing growth increments for the crack front points.");
  params.addParam<ReporterValueName>(
      "time_to_max_growth_size_name",
      "max_growth_timestep",
      "Reporter name containing the timestep to reach max_growth_size.");

  // these flags must must match those used by the InteractionIntegral set in the
  // DomainIntegralAction
  ExecFlagEnum & exec = params.set<ExecFlagEnum>("execute_on");
  exec.addAvailableFlags(EXEC_XFEM_MARK);
  params.setDocString("execute_on", exec.getDocString());
  params.set<ExecFlagEnum>("execute_on") = {EXEC_XFEM_MARK, EXEC_TIMESTEP_END};
  return params;
}

StressCorrosionCracking::StressCorrosionCracking(const InputParameters & parameters)
  : GeneralReporter(parameters),
    _cutter_name(getParam<UserObjectName>("crackMeshCut3DUserObject_name")),
    _3Dcutter(&_fe_problem.getUserObject<CrackMeshCut3DUserObject>(_cutter_name)),
    _max_growth_size(getParam<Real>("max_growth_size")),
    _k_low(getParam<Real>("k_low")),
    _growth_rate_low(getParam<Real>("growth_rate_low")),
    _k_high(getParam<Real>("k_high")),
    _growth_rate_high(getParam<Real>("growth_rate_high")),
    _growth_rate_mid_multiplier(getParam<Real>("growth_rate_mid_multiplier")),
    _growth_rate_mid_exp_factor(getParam<Real>("growth_rate_mid_exp_factor")),
    _ki_vpp(getVectorPostprocessorValue(
        "ki_vectorpostprocessor", getParam<VectorPostprocessorName>("ki_vectorpostprocessor"))),
    _ki_x(getVectorPostprocessorValue("ki_vectorpostprocessor", "x")),
    _ki_y(getVectorPostprocessorValue("ki_vectorpostprocessor", "y")),
    _ki_z(getVectorPostprocessorValue("ki_vectorpostprocessor", "z")),
    _ki_id(getVectorPostprocessorValue("ki_vectorpostprocessor", "id")),
    _corrosion_time_step(declareValueByName<Real>(
        getParam<ReporterValueName>("time_to_max_growth_size_name"), REPORTER_MODE_ROOT)),
    _growth_increment(declareValueByName<std::vector<Real>>(
        getParam<ReporterValueName>("growth_increment_name"), REPORTER_MODE_ROOT)),
    _x(declareValueByName<std::vector<Real>>("x", REPORTER_MODE_ROOT)),
    _y(declareValueByName<std::vector<Real>>("y", REPORTER_MODE_ROOT)),
    _z(declareValueByName<std::vector<Real>>("z", REPORTER_MODE_ROOT)),
    _id(declareValueByName<std::vector<Real>>("id", REPORTER_MODE_ROOT))
{
}

void
StressCorrosionCracking::execute()
{
  _x.resize(_ki_x.size());
  _y.resize(_ki_y.size());
  _z.resize(_ki_z.size());
  _id.resize(_ki_id.size());
  std::copy(_ki_x.begin(), _ki_x.end(), _x.begin());
  std::copy(_ki_y.begin(), _ki_y.end(), _y.begin());
  std::copy(_ki_z.begin(), _ki_z.end(), _z.begin());
  std::copy(_ki_id.begin(), _ki_id.end(), _id.begin());

  _growth_increment.resize(_ki_x.size());

  // Generate _active_boundary and _inactive_boundary_pos;
  // This is a duplicated call before the one in CrackMeshCut3DUserObject;
  // That one cannot be deleted because this one is for subcritical cracking only
  // the test still pass so lynn is not sure if this is actually needed but it was
  // in the original post processor so I'll keep it, even though it is non const.
  _3Dcutter->findActiveBoundaryNodes();
  std::vector<int> index = _3Dcutter->getFrontPointsIndex();

  std::vector<Real> growth_rate(_ki_x.size(), 0.0);
  for (std::size_t i = 0; i < _ki_vpp.size(); ++i)
  {
    int ind = index[i];
    if (ind == -1)
      growth_rate[i] = 0;
    else
    {
      if (_ki_vpp[ind] < _k_low)
        growth_rate[i] = _growth_rate_low;
      else if ((_ki_vpp[ind] >= _k_low) && (_ki_vpp[ind] < _k_high))
        growth_rate[i] =
            _growth_rate_mid_multiplier * std::pow(_ki_vpp[ind], _growth_rate_mid_exp_factor);
      else
        growth_rate[i] = _growth_rate_high;
    }
  }

  Real max_growth_rate = *std::max_element(growth_rate.begin(), growth_rate.end());
  _corrosion_time_step = _max_growth_size / max_growth_rate;

  for (std::size_t i = 0; i < _ki_vpp.size(); ++i)
  {
    int ind = index[i];
    if (ind == -1)
      _growth_increment[i] = 0.0;
    else
      _growth_increment[i] = growth_rate[i] * _corrosion_time_step;
  }
}
