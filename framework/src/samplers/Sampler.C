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

  // Define the allowable limits for data returned by getSamples/getLocalSamples/getNextLocalRow
  // to prevent system for going over allowable limits. The DenseMatrix object uses unsigned int
  // for size definition, so as start the limits will be based at 10% the max of unsigned int. Note,
  // the values here are the limits of the number of items in the complete container. dof_id_type
  // is used just in case in the future we need more.
  params.addParam<dof_id_type>(
      "limit_get_samples",
      0.1 * std::numeric_limits<unsigned int>::max(),
      "The maximum allowed number of items in the DenseMatrix returned by getSamples method.");
  params.addParam<dof_id_type>(
      "limit_get_local_samples",
      0.1 * std::numeric_limits<unsigned int>::max(),
      "The maximum allowed number of items in the DenseMatrix returned by getSamples method.");
  params.addParam<dof_id_type>(
      "limit_get_next_local_row",
      0.1 * std::numeric_limits<unsigned int>::max(),
      "The maximum allowed number of items in the DenseMatrix returned by getSamples method.");
  return params;
}

Sampler::Sampler(const InputParameters & parameters)
  : MooseObject(parameters),
    SetupInterface(this),
    DistributionInterface(this),
    _seed(getParam<unsigned int>("seed")),
    _n_rows(0),
    _n_cols(0),
    _next_local_row_requires_state_restore(true),
    _initialized(false),
    _limit_get_samples(getParam<dof_id_type>("limit_get_samples")),
    _limit_get_local_samples(getParam<dof_id_type>("limit_get_local_samples")),
    _limit_get_next_local_row(getParam<dof_id_type>("limit_get_next_local_row"))
{
  setNumberOfRandomSeeds(1);
}

void
Sampler::init()
{
  // The init() method is private so it is un-likely to be called, but just in case the following
  // was added to help avoid future mistakes.
  if (_initialized)
    mooseError("The Sampler::init() method is called automatically and should not be called.");

  if (_n_rows == 0)
    mooseError("The number of rows cannot be zero.");

  if (_n_cols == 0)
    mooseError("The number of columns cannot be zero.");

  // TODO: If Sampler is updated to be threaded, this partitioning must also include threads
  MooseUtils::linearPartitionItems(
      _n_rows, n_processors(), processor_id(), _n_local_rows, _local_row_begin, _local_row_end);

  // See FEProblemBase::execute
  execute();

  // Set the next row iterator index
  _next_local_row = _local_row_begin;

  _initialized = true;
}

void
Sampler::setNumberOfRows(dof_id_type n_rows)
{
  if (_initialized)
    mooseError(
        "The 'setNumberOfRows()' method can not be called after the Sampler has been initialized; "
        "this method should be called in the constructor of the Sampler object.");

  _n_rows = n_rows;
}

void
Sampler::setNumberOfCols(dof_id_type n_cols)
{
  if (_initialized)
    mooseError(
        "The 'setNumberOfCols()' method can not be called after the Sampler has been initialized; "
        "this method should be called in the constructor of the Sampler object.");

  _n_cols = n_cols;
}

void
Sampler::execute()
{
  _generator.saveState();
}

DenseMatrix<Real>
Sampler::getSamples()
{
  if (_n_rows * _n_cols > _limit_get_samples)
    paramError("limit_get_samples",
               "The number of entries in the DenseMatrix (",
               _n_rows * _n_cols,
               ") exceeds the allowed limit of ",
               _limit_get_samples,
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
  for (dof_id_type j = 0; j < _n_cols; ++j)
    output[j] = computeSample(_next_local_row, j);
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
  for (dof_id_type i = 0; i < _n_rows; ++i)
    for (dof_id_type j = 0; j < _n_cols; ++j)
      matrix(i, j) = computeSample(i, j);
}

void
Sampler::computeLocalSampleMatrix(DenseMatrix<Real> & matrix)
{
  advanceGenerators(_local_row_begin * _n_cols);
  for (dof_id_type i = _local_row_begin; i < _local_row_end; ++i)
    for (dof_id_type j = 0; j < _n_cols; ++j)
      matrix(i - _local_row_begin, j) = computeSample(i, j);
  advanceGenerators((_n_rows - _local_row_end) * _n_cols);
}

void
Sampler::advanceGenerators(dof_id_type count)
{
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

void
Sampler::setNumberOfRandomSeeds(std::size_t number)
{
  if (number == 0)
    mooseError("The number of seeds must be larger than zero.");

  if (_initialized)
    mooseError("The 'setNumberOfRandomSeeds()' method can not be called after the Sampler has been "
               "initialized; "
               "this method should be called in the constructor of the Sampler object.");

  // Seed the "master" seed generator
  _seed_generator.seed(0, _seed);

  // See the "slave" generator that will be used for the random number generation
  for (std::size_t i = 0; i < number; ++i)
    _generator.seed(i, _seed_generator.randl(0));
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
