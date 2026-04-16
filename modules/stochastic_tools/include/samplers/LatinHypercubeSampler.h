//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Sampler.h"

// Forward declarations
class MooseRandomPerturbation;

/**
 * Implements Latin Hypercube Sampling (LHS) over a set of distributions.
 *
 * For N samples and K distributions the output matrix has N rows and K columns.
 * LHS divides the [0,1] probability axis into N equal-width bins and ensures
 * that exactly one sample falls in each bin for every column. Within each bin
 * the sample point is drawn uniformly at random, and the bin assignment for
 * each column is an independent pseudo-random permutation of [0, N) seeded
 * from generator 1 during executeSetUp().
 *
 * Generator usage:
 *  - Generator 0: within-bin uniform draws (one per computeSample call).
 *  - Generator 1: column shuffler seeds (one per column, drawn in executeSetUp).
 */
class LatinHypercubeSampler : public Sampler
{
public:
  static InputParameters validParams();

  LatinHypercubeSampler(const InputParameters & parameters);

protected:
  /**
   * Constructs one MooseRandomPerturbation per column, each seeded
   * independently from generator 1.
   */
  virtual void executeTearDown() override;

  /**
   * Return the sample value for the given row and column.
   *
   * Maps \p row_index to a shuffled bin index via the column's permuter,
   * draws a uniform random point within that bin, then transforms it through
   * the column distribution's quantile function.
   *
   * @param row_index  Global row index in [0, getNumberOfRows()).
   * @param col_index  Column index in [0, getNumberOfCols()).
   * @return           A sample drawn from distributions[col_index] respecting
   *                   the LHS bin constraint for this row.
   */
  virtual Real computeSample(dof_id_type row_index, dof_id_type col_index) override;

  /// Distribution objects, one per column, whose quantile functions are sampled
  std::vector<Distribution const *> _distributions;
  /// Per-column pseudo-random permuters that enforce the LHS bin assignment
  std::vector<std::unique_ptr<MooseRandomPerturbation>> _shufflers;
};
