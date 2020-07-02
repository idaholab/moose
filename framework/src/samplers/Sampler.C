//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// STL includes
#include <iterator>

// MOOSE includes
#include "Sampler.h"

defineLegacyParams(Sampler);

InputParameters
Sampler::validParams()
{
  InputParameters params = MooseObject::validParams();
  params += SetupInterface::validParams();
  params += DistributionInterface::validParams();
  params.addClassDescription("A base class for distribution sampling.");

  ExecFlagEnum & exec_enum = params.set<ExecFlagEnum>("execute_on", true);
  exec_enum.addAvailableFlags(EXEC_PRE_MULTIAPP_SETUP);

  params.addParam<unsigned int>("seed", 0, "Random number generator initial seed");
  params.registerBase("Sampler");
  params.registerSystemAttributeName("Sampler");

  params.addParam<bool>("legacy_support", true, "Disables errors for legacy API support.");

  // Define the allowable limits for data returned by getSamples/getLocalSamples/getNextLocalRow
  // to prevent system for going over allowable limits. The DenseMatrix object uses unsigned int
  // for size definition, so as start the limits will be based the max of unsigned int. Note,
  // the values here are the limits of the number of items in the complete container. dof_id_type
  // is used just in case in the future we need more.
  params.addParam<dof_id_type>("limit_get_global_samples",
                               0.1 * std::numeric_limits<unsigned int>::max(),
                               "The maximum allowed number of items in the DenseMatrix returned by "
                               "getGlobalSamples method.");
  params.addParam<dof_id_type>(
      "limit_get_local_samples",
      0.1 * std::numeric_limits<unsigned int>::max(),
      "The maximum allowed number of items in the DenseMatrix returned by getLocalSamples method.");
  params.addParam<dof_id_type>(
      "limit_get_next_local_row",
      0.1 * std::numeric_limits<unsigned int>::max(),
      "The maximum allowed number of items in the std::vector returned by getNextLocalRow method.");
  return params;
}

Sampler::Sampler(const InputParameters & parameters)
  : MooseObject(parameters),
    SetupInterface(this),
    DistributionInterface(this),
    PerfGraphInterface(this),
    SamplerInterface(this),
    _n_rows(0),
    _n_cols(0),
    _n_seeds(1),
    _next_local_row_requires_state_restore(true),
    _initialized(false),
    _has_executed(false),
    _limit_get_global_samples(getParam<dof_id_type>("limit_get_global_samples")),
    _limit_get_local_samples(getParam<dof_id_type>("limit_get_local_samples")),
    _limit_get_next_local_row(getParam<dof_id_type>("limit_get_next_local_row")),
    _perf_get_global_samples(registerTimedSection("getGlobalSamples", 1)),
    _perf_get_local_samples(registerTimedSection("getLocalSamples", 1)),
    _perf_get_next_local_row(registerTimedSection("getNextLocalRow", 1)),
    _perf_sample_row(registerTimedSection("computeSampleRow", 2)),
    _perf_local_sample_matrix(registerTimedSection("computeLocalSampleMatrix", 2)),
    _perf_sample_matrix(registerTimedSection("computeSampleMatrix", 2)),
    _perf_advance_generator(registerTimedSection("advanceGenerators", 2))
{
}

void
Sampler::init()
{
  // The init() method is private so it is un-likely to be called, but just in case the following
  // was added to help avoid future mistakes.
  if (_initialized)
    mooseError("The Sampler::init() method is called automatically and should not be called.");

  // TODO: If Sampler is updated to be threaded, this partitioning must also include threads
  MooseUtils::linearPartitionItems(
      _n_rows, n_processors(), processor_id(), _n_local_rows, _local_row_begin, _local_row_end);

  // Set the next row iterator index
  _next_local_row = _local_row_begin;

  // Seed the "master" seed generator
  const unsigned int seed = getParam<unsigned int>("seed");
  MooseRandom seed_generator;
  seed_generator.seed(0, seed);

  // See the "secondary" generator that will be used for the random number generation
  for (std::size_t i = 0; i < _n_seeds; ++i)
    _generator.seed(i, seed_generator.randl(0));

  // Save the initial state
  _generator.saveState();

  // Mark class as initialized, which locks out certain methods
  _initialized = true;
}

void
Sampler::setNumberOfRows(dof_id_type n_rows)
{
  if (_initialized)
    mooseError(
        "The 'setNumberOfRows()' method can not be called after the Sampler has been initialized; "
        "this method should be called in the constructor of the Sampler object.");

  if (n_rows == 0)
    mooseError("The number of rows cannot be zero.");

  _n_rows = n_rows;
}

void
Sampler::setNumberOfCols(dof_id_type n_cols)
{
  if (_initialized)
    mooseError(
        "The 'setNumberOfCols()' method can not be called after the Sampler has been initialized; "
        "this method should be called in the constructor of the Sampler object.");

  if (n_cols == 0)
    mooseError("The number of columns cannot be zero.");

  _n_cols = n_cols;
}

void
Sampler::setNumberOfRandomSeeds(std::size_t n_seeds)
{
  if (_initialized)
    mooseError("The 'setNumberOfRandomSeeds()' method can not be called after the Sampler has been "
               "initialized; "
               "this method should be called in the constructor of the Sampler object.");

  if (n_seeds == 0)
    mooseError("The number of seeds must be larger than zero.");

  _n_seeds = n_seeds;
}

void
Sampler::execute()
{
  if (_has_executed)
  {
    _generator.restoreState();
    advanceGenerators(_n_rows * _n_cols);
  }
  _generator.saveState();
  _has_executed = true;
}

DenseMatrix<Real>
Sampler::getGlobalSamples()
{
  TIME_SECTION(_perf_get_global_samples);

  if (_n_rows * _n_cols > _limit_get_global_samples)
    paramError("limit_get_global_samples",
               "The number of entries in the DenseMatrix (",
               _n_rows * _n_cols,
               ") exceeds the allowed limit of ",
               _limit_get_global_samples,
               ".");

  _next_local_row_requires_state_restore = true;
  _generator.restoreState();
  sampleSetUp();
  DenseMatrix<Real> output(_n_rows, _n_cols);
  computeSampleMatrix(output);
  sampleTearDown();
  return output;
}

DenseMatrix<Real>
Sampler::getLocalSamples()
{
  TIME_SECTION(_perf_get_local_samples);

  if (_n_local_rows * _n_cols > _limit_get_local_samples)
    paramError("limit_get_local_samples",
               "The number of entries in the DenseMatrix (",
               _n_local_rows * _n_cols,
               ") exceeds the allowed limit of ",
               _limit_get_local_samples,
               ".");

  _next_local_row_requires_state_restore = true;
  _generator.restoreState();
  sampleSetUp();
  DenseMatrix<Real> output(_n_local_rows, _n_cols);
  computeLocalSampleMatrix(output);
  sampleTearDown();
  return output;
}

std::vector<Real>
Sampler::getNextLocalRow()
{
  TIME_SECTION(_perf_get_next_local_row);

  if (_next_local_row_requires_state_restore)
  {
    _generator.restoreState();
    sampleSetUp();
    advanceGenerators(_next_local_row * _n_cols);
    _next_local_row_requires_state_restore = false;

    if (_n_cols > _limit_get_next_local_row)
      paramError("limit_get_next_local_row",
                 "The number of entries in the std::vector (",
                 _n_cols,
                 ") exceeds the allowed limit of ",
                 _limit_get_next_local_row,
                 ".");
  }

  std::vector<Real> output(_n_cols);
  computeSampleRow(_next_local_row, output);
  mooseAssert(output.size() == _n_cols, "The row of sample data is not sized correctly.");
  _next_local_row++;

  if (_next_local_row == _local_row_end)
  {
    advanceGenerators((_n_rows - _local_row_end) * _n_cols);
    sampleTearDown();
    _next_local_row = _local_row_begin;
    _next_local_row_requires_state_restore = true;
  }

  return output;
}

void
Sampler::computeSampleMatrix(DenseMatrix<Real> & matrix)
{
  TIME_SECTION(_perf_sample_matrix);

  for (dof_id_type i = 0; i < _n_rows; ++i)
  {
    std::vector<Real> row(_n_cols, 0);
    computeSampleRow(i, row);
    mooseAssert(row.size() == _n_cols, "The row of sample data is not sized correctly.");
    std::copy(row.begin(), row.end(), matrix.get_values().begin() + i * _n_cols);
  }
}

void
Sampler::computeLocalSampleMatrix(DenseMatrix<Real> & matrix)
{
  TIME_SECTION(_perf_local_sample_matrix);

  advanceGenerators(_local_row_begin * _n_cols);
  for (dof_id_type i = _local_row_begin; i < _local_row_end; ++i)
  {
    std::vector<Real> row(_n_cols, 0);
    computeSampleRow(i, row);
    mooseAssert(row.size() == _n_cols, "The row of sample data is not sized correctly.");
    std::copy(
        row.begin(), row.end(), matrix.get_values().begin() + ((i - _local_row_begin) * _n_cols));
  }
  advanceGenerators((_n_rows - _local_row_end) * _n_cols);
}

void
Sampler::computeSampleRow(dof_id_type i, std::vector<Real> & data)
{
  TIME_SECTION(_perf_sample_row);

  for (dof_id_type j = 0; j < _n_cols; ++j)
    data[j] = computeSample(i, j);
}

void
Sampler::advanceGenerators(dof_id_type count)
{
  TIME_SECTION(_perf_advance_generator);

  for (dof_id_type i = 0; i < count; ++i)
    for (std::size_t j = 0; j < _generator.size(); ++j)
      getRand(j);
}

double
Sampler::getRand(const unsigned int index)
{
  mooseAssert(index < _generator.size(), "The seed number index does not exists.");
  return _generator.rand(index);
}

uint32_t
Sampler::getRandl(unsigned int index, uint32_t lower, uint32_t upper)
{
  mooseAssert(index < _generator.size(), "The seed number index does not exists.");
  return _generator.randl(index, lower, upper);
}

dof_id_type
Sampler::getNumberOfRows() const
{
  return _n_rows;
}

dof_id_type
Sampler::getNumberOfCols() const
{
  return _n_cols;
}

dof_id_type
Sampler::getNumberOfLocalRows() const
{
  return _n_local_rows;
}

dof_id_type
Sampler::getLocalRowBegin() const
{
  return _local_row_begin;
}

dof_id_type
Sampler::getLocalRowEnd() const
{
  return _local_row_end;
}

// DEPRECATED: Everything below should removed when apps are updated to new syntax
std::vector<DenseMatrix<Real>>
Sampler::sample()
{
  return std::vector<DenseMatrix<Real>>();
}

std::vector<DenseMatrix<Real>>
Sampler::getSamples()
{
  mooseDoOnce(mooseDeprecated(
      "getSamples is being removed, use getNextLocalRow, getLocalSamples, or getGlobalSamples."));

  _generator.restoreState();
  sampleSetUp();
  std::vector<DenseMatrix<Real>> output = sample();
  sampleTearDown();

  mooseAssert(output.size() > 0,
              "It is not acceptable to return an empty vector of sample matrices.");

  return output;
}

// TODO: Remove this and restore to pure virtual when deprecated syntax is removed
Real Sampler::computeSample(dof_id_type, dof_id_type) { return 0.0; }

double
Sampler::rand(const unsigned int index)
{
  mooseDoOnce(mooseDeprecated("rand(() is being removed, use getRand()"));
  mooseAssert(index < _generator.size(), "The seed number index does not exists.");
  return _generator.rand(index);
}
