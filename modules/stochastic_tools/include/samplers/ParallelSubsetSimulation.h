//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Sampler.h"
#include "ReporterInterface.h"

/**
 * A class used to perform Parallel Subset Simulation Sampling
 */
class ParallelSubsetSimulation : public Sampler, public ReporterInterface
{
public:
  static InputParameters validParams();

  ParallelSubsetSimulation(const InputParameters & parameters);

protected:
  /// Return the sample for the given row and column
  virtual Real computeSample(dof_id_type row_index, dof_id_type col_index) override;

  /// Storage for distribution objects to be utilized
  std::vector<Distribution const *> _distributions;

  /// Distribution names
  const std::vector<DistributionName> & _distribution_names;

  // const std::vector<ReporterName> & _inputs_names;

  const int & _num_samplessub;

  const bool & _use_absolute_value;
  const Real & _subset_probability;

  std::vector<std::vector<Real>> _inputs_sto;

  std::vector<Real> _outputs_sto;

private:

  // std::vector<Real> _data_rep;

  // const std::vector<Real> & _vector;

  // std::vector<Real> _new_sample_vec;
  std::vector<std::vector<Real>> _new_sample_vec;

  std::vector<Real> _proposal_std;

  Real count2;

  Real _acceptance_ratio;

  std::vector<const VectorPostprocessorValue *> _values_ptr;

  const int & _step;

  unsigned int _subset;

  unsigned int _seed_value;

  int _ind_sto;
  // std::vector<Real> _markov_seed;
  std::vector<std::vector<Real>> _markov_seed;
  unsigned int _count;
  int _check_even;
  unsigned int _count_max;
  std::vector<Real> _output_sorted;
  std::vector<std::vector<Real>> _inputs_sorted;

  /// Storage of the previous sample to propose the next sample
  std::vector<Real> _prev_val;

  std::vector<Real> _output_limits;

  /// PerfGraph timer
  const PerfID _perf_compute_sample;
};
