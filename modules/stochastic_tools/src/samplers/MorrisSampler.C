//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MorrisSampler.h"
#include "Distribution.h"

registerMooseObject("StochasticToolsApp", MorrisSampler);

InputParameters
MorrisSampler::validParams()
{
  InputParameters params = Sampler::validParams();
  params.addClassDescription("Morris variance-based sensitivity analysis Sampler.");
  params.addRequiredParam<std::vector<DistributionName>>(
      "distributions",
      "The distribution names to be sampled, the number of distributions provided defines the "
      "number of columns per matrix.");
  params.addRequiredRangeCheckedParam<dof_id_type>(
      "trajectories",
      "trajectories > 0",
      "Number of unique trajectories to perform. The higher number of these usually means a more "
      "accurate sensitivity evaluation, but it is proportional to the number of required model "
      "evaluations: 'trajectoris' x (number of 'distributions' + 1).");
  params.addRangeCheckedParam<unsigned int>(
      "levels",
      4,
      "levels % 2 = 0 & levels > 0",
      "The number of levels in the sampling. This determines the discretization of the input "
      "space, more levels means finer discretization and more possible model perturbations.");
  return params;
}

MorrisSampler::MorrisSampler(const InputParameters & parameters)
  : Sampler(parameters),
    _num_trajectories(getParam<dof_id_type>("trajectories")),
    _num_levels(getParam<unsigned int>("levels"))

{
  for (const auto & name : getParam<std::vector<DistributionName>>("distributions"))
    _distributions.push_back(&getDistributionByName(name));

  setNumberOfCols(_distributions.size());
  setNumberOfRows(_num_trajectories * (_distributions.size() + 1));
}

void
MorrisSampler::sampleSetUp(const Sampler::SampleMode /*mode*/)
{
  const dof_id_type nc = getNumberOfCols();
  _b = RealEigenMatrix::Ones(nc + 1, nc).triangularView<Eigen::StrictlyLower>();
  _pstar.resize(nc, nc);
  _j.setOnes(nc + 1, nc);
  _dstar.resize(nc, nc);
  _xstar.resize(nc + 1, nc);
  _bstar.resize(nc + 1, nc);
}

Real
MorrisSampler::computeSample(dof_id_type row_index, dof_id_type col_index)
{
  const dof_id_type traj_ind = row_index % (getNumberOfCols() + 1);
  if (traj_ind == 0 && col_index == 0)
    updateBstar();
  return _distributions[col_index]->quantile(_bstar(traj_ind, col_index));
}

void
MorrisSampler::updateBstar()
{
  const dof_id_type nc = getNumberOfCols(); // convenience
  _pstar.setZero();
  // Which parameter to perturb
  std::vector<dof_id_type> pchoice(nc);
  std::iota(pchoice.begin(), pchoice.end(), 0);
  for (dof_id_type c = 0; c < nc; ++c)
  {
    const unsigned int ind = nc > 1 ? getRandl(0, 0, pchoice.size()) : 0;
    _pstar(pchoice[ind], c) = 1.0;
    pchoice.erase(pchoice.begin() + ind);
  }

  _dstar.setZero();
  // Direction of perturbation
  for (dof_id_type c = 0; c < nc; ++c)
    _dstar(c, c) = getRand() < 0.5 ? -1.0 : 1.0;

  // Initial value
  for (dof_id_type c = 0; c < nc; ++c)
  {
    const auto lind = getRandl(0, 0, _num_levels / 2);
    _xstar.col(c).setConstant((Real)lind * 1.0 / ((Real)_num_levels - 1));
  }

  _bstar =
      _xstar + _num_levels / 4.0 / (_num_levels - 1) * ((2.0 * _b * _pstar - _j) * _dstar + _j);

  // This matrix represent _n_cols * (_n_cols + 1) samples, but so far we have only
  // advanced the generator 3 * _n_cols times. For the generator state restore
  // to work properly, we need to finish advancing the generator
  if (nc > 2)
    advanceGenerator(0, nc * (nc - 2));
}

LocalRankConfig
MorrisSampler::constructRankConfig(bool batch_mode) const
{
  std::vector<LocalRankConfig> all_rc(processor_id() + 1);
  for (processor_id_type r = 0; r <= processor_id(); ++r)
    all_rc[r] = rankConfig(
        r, n_processors(), _num_trajectories, _min_procs_per_row, _max_procs_per_row, batch_mode);
  LocalRankConfig & rc = all_rc.back();

  rc.num_local_sims *= _distributions.size() + 1;
  bool found_first = false;
  for (auto it = all_rc.rbegin(); it != all_rc.rend(); ++it)
    if (it->is_first_local_rank)
    {
      if (found_first)
        rc.first_local_sim_index += it->num_local_sims * _distributions.size();
      else
        found_first = true;
    }

  if (!batch_mode)
  {
    rc.num_local_apps = rc.num_local_sims;
    rc.first_local_app_index = rc.first_local_sim_index;
  }

  return rc;
}
