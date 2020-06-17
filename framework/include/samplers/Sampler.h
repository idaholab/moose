//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "DenseMatrix.h"
#include "MooseObject.h"
#include "MooseRandom.h"
#include "SetupInterface.h"
#include "DistributionInterface.h"
#include "PerfGraphInterface.h"
#include "SamplerInterface.h"

template <>
InputParameters validParams<Sampler>();

/**
 * This is the base class for Samplers as used within the Stochastic Tools module.
 *
 * A sampler is responsible for sampling distributions and providing an API for providing
 * the sample data to objects. The sampler is designed to handle any number of random number
 * generators.
 *
 * The main methods in this object is the getNextLocalRow/getSamples/getLocalSamples methods which
 * return the distribution samples. These methods will return the same results for each call
 * regardless of the number of calls, with getNextLocalRow being the exception because it is
 * designed be used in an iterative approach.
 *
 * Samplers support the use of "execute_on", which when called results in new set of random numbers,
 * thus after execute() calls to getSamples/getLocalSamples() methods will now produce a new set of
 * random numbers from calls prior to the execute() call.
 *
 * Not for MOOSE developers: Great care was taken to design the structure of this class to limit
 * access to the critical portions of this object while making it extensible. Please consider
 * carefully the impacts of altering the API.
 */
class Sampler : public MooseObject,
                public SetupInterface,
                public DistributionInterface,
                public PerfGraphInterface,
                public SamplerInterface
{
public:
  static InputParameters validParams();

  Sampler(const InputParameters & parameters);

  // DEPRECATED, DO NOT USE
  virtual std::vector<DenseMatrix<Real>> sample();
  std::vector<DenseMatrix<Real>> getSamples();
  double rand(unsigned int index = 0);

  // The public members define the API that is exposed to application developers that are using
  // Sampler objects to perform calculations, so be very careful when adding items here since
  // they are exposed to any other object via the SamplerInterface.
  //
  // It is also important to point out that when Sampler objects, when used, are not const. This is
  // due to the fact that calling the various get methods below must store various pieces of data as
  // well as control the state of the random number generators.

  ///@{
  /**
   * Return the sampled complete or distributed sample data.
   *
   * Use these with caution as they can result in a large amount of memory use, the preferred
   * method for accessing Sampler data is the getNextLocalRow() method.
   */
  DenseMatrix<Real> getGlobalSamples();
  DenseMatrix<Real> getLocalSamples();
  ///@}

  /**
   * Return the "next" local row. This is designed to be used within a loop using the
   * getLocalRowBegin/End methods as such:
   *
   * for (dof_id_type i = getLocalRowBegin(); i < getLocalRowEnd(); ++i)
   *     std::vector<Real> row = getNextLocalRow();
   *
   * This is the preferred method for accessing Sampler data.
   *
   * Calls to getNextLocalRow() will continue to return the next row of data until the last local
   * row has been reached, it will then start again at the beginning if called again. Also, calls
   * to getNextLocalRow() can be partial, followed by call(s) to getSamples or getLocalSamples.
   * Continued calls to getNextLocalRow() will still continue to give the next row as if the
   * other get calls were not made. However, when this occurs calls to restore and advance the
   * generators are made after each call to getSamples or getLocalSamples, so this generally
   * should be avoided.
   */
  std::vector<Real> getNextLocalRow();

  /**
   * Return the number of samples.
   * @return The total number of rows that exist in all DenseMatrix values from the
   * getSamples/getLocalSamples methods.
   */
  dof_id_type getNumberOfRows() const;
  dof_id_type getNumberOfCols() const;
  dof_id_type getNumberOfLocalRows() const;

  ///@{
  /**
   * Return the beginning/end local row index for this processor
   */
  dof_id_type getLocalRowBegin() const;
  dof_id_type getLocalRowEnd() const;
  ///@}

protected:
  // The following methods are the basic methods that should be utilized my most application
  // developers that are creating a custom Sampler.

  ///@{
  /**
   * These methods must be called within the constructor of child classes to define the size of
   * the matrix to be created.
   */
  void setNumberOfRows(dof_id_type n_rows);
  void setNumberOfCols(dof_id_type n_cols);
  ///@}

  /**
   * Set the number of seeds required by the sampler. The Sampler will generate
   * additional seeds as needed. This function should be called in the constructor
   * of child objects.
   * @param number The required number of random seeds, by default this is called with 1.
   */
  void setNumberOfRandomSeeds(std::size_t number);

  /**
   * Get the next random number from the generator.
   * @param index The index of the seed, by default this is zero. To add additional seeds
   *              indices call the setNumberOfRequiedRandomSeeds method.
   *
   * @return A double for the random number, this is double because MooseRandom class uses double.
   */
  Real getRand(unsigned int index = 0);

  /**
   * Get the next random integer from the generator within the specified range [lower, upper)
   * @param index The index of the seed, by default this is zero. To add additional seeds
   *              indices call the setNumberOfRequiedRandomSeeds method.
   * @param lower Lower bounds
   * @param upper Upper bounds
   *
   * @return A integer for the random number
   */
  uint32_t getRandl(unsigned int index, uint32_t lower, uint32_t upper);

  // TODO: Restore this pure virtual after application are updated to new interface
  /**
   * Base class must override this method to supply the sample distribution data.
   * @param row_index The row index of sample value to compute.
   * @param col_index The column index of sample value to compute.
   * @return The value for the given row and column.
   */
  virtual Real computeSample(dof_id_type row_index, dof_id_type col_index);

  ///@{
  /**
   * Setup method called prior and after looping through distributions.
   *
   * These methods should not be called directly, each is automatically called by the public
   * getGlobalSamples() or getLocalSamples() methods.
   */
  virtual void sampleSetUp(){};
  virtual void sampleTearDown(){};
  ///@}

  // The following methods are advanced methods that should not be needed by application developers,
  // but exist for special cases.

  ///@{
  /**
   * Methods to populate the global or local sample matrix.
   * @param matrix The correctly sized matrix of sample values to populate
   *
   * These methods should not be called directly, each is automatically called by the public
   * getGlobalSamples() or getLocalSamples() methods.
   */
  virtual void computeSampleMatrix(DenseMatrix<Real> & matrix);
  virtual void computeLocalSampleMatrix(DenseMatrix<Real> & matrix);
  ///@}

  ///@{
  /**
   * Method to populate a complete row of sample data.
   * @param i The global row index to compute
   * @param data The correctly sized vector of sample value to poplulate

   * This method should not be called directly, it is automatically called by the public
   * getGlobalSamples(), getLocalSamples(), or getNextLocalRow() methods.
   */
  virtual void computeSampleRow(dof_id_type i, std::vector<Real> & data);

  /**
   * Method for advancing the random number generator(s) by the supplied number or calls to rand().
   *
   * TODO: This should be updated if the If the random number generator is updated to type that
   * supports native advancing.
   */
  virtual void advanceGenerators(dof_id_type count);

private:
  /**
   * Function called by MOOSE to setup the Sampler for use. The primary purpose is to partition
   * the DenseMatrix rows for parallel distribution. A separate method is required so that the
   * set methods can be called within the constructors of child objects, see
   * FEProblemBase::addSampler method.
   *
   * This init() method is called by FEProblemBase::addSampler; it should not be called elsewhere.
   */
  void init();
  friend void FEProblemBase::addSampler(const std::string & type,
                                        const std::string & name,
                                        InputParameters & parameters);

  /**
   * Store the state of the MooseRandom generator so that new calls to
   * getGlobalSamples/getLocalSamples methods will create new numbers.
   *
   * The execute() method is called in the init() method of this class and
   * FEProblemBase::executeSamplers; it should not be called elsewhere.
   */
  void execute();
  friend void FEProblemBase::objectExecuteHelper<Sampler>(const std::vector<Sampler *> & objects);

  /// Random number generator, don't give users access. Control it via the interface from this class.
  MooseRandom _generator;

  /// Number of rows for this processor
  dof_id_type _n_local_rows;

  /// Global row index for start of data for this processor
  dof_id_type _local_row_begin;

  /// Global row index for end of data for this processor
  dof_id_type _local_row_end;

  /// Total number of rows in the sample matrix
  dof_id_type _n_rows;

  /// Total number of columns in the sample matrix
  dof_id_type _n_cols;

  /// Number of seeds
  std::size_t _n_seeds;

  /// Iterator index for getNextLocalRow method
  dof_id_type _next_local_row;

  /// Flag for restoring state during getNextLocalRow iteration
  bool _next_local_row_requires_state_restore;

  /// Flag to indicate if the init method for this class was called
  bool _initialized;

  /// Flag for initial execute to allow the first set of random numbers to be always be the same
  bool _has_executed;

  /// Max number of entries for matrix returned by getGlobalSamples
  const dof_id_type _limit_get_global_samples;

  /// Max number of entries for matrix returned by getLocalSamples
  const dof_id_type _limit_get_local_samples;

  /// Max number of entries for matrix returned by getNextLocalRow
  const dof_id_type _limit_get_next_local_row;

  ///@{
  /// PrefGraph timers
  const PerfID _perf_get_global_samples;
  const PerfID _perf_get_local_samples;
  const PerfID _perf_get_next_local_row;
  const PerfID _perf_sample_row;
  const PerfID _perf_local_sample_matrix;
  const PerfID _perf_sample_matrix;
  const PerfID _perf_advance_generator;
  ///@}
};
