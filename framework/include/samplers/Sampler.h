//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Shuffle.h"
#include "DenseMatrix.h"
#include "MooseObject.h"
#include "MooseRandom.h"
#include "SetupInterface.h"
#include "DistributionInterface.h"
#include "PerfGraphInterface.h"
#include "SamplerInterface.h"
#include "MultiApp.h"
#include "VectorPostprocessorInterface.h"
#include "ReporterInterface.h"
#include "MooseRandomStateless.h"

/**
 * This is the base class for Samplers as used within the Stochastic Tools module.
 *
 * A sampler is responsible for sampling distributions and providing an API for providing
 * the sample data to objects. The sampler is designed to handle any number of random number
 * generators.
 *
 * The main methods in this object is the getSampleRow/getLocalSamples methods which
 * return the distribution samples. These methods will return the same results for each call
 * regardless of the number of calls.
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
                public SamplerInterface,
                public VectorPostprocessorInterface,
                public ReporterInterface
{
public:
  enum class SampleMode
  {
    GLOBAL = 0,
    LOCAL = 1
  };

  static InputParameters validParams();
  Sampler(const InputParameters & parameters);

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
   * method for accessing Sampler data is the getSampleRow() method.
   */
  DenseMatrix<Real> getGlobalSamples();
  DenseMatrix<Real> getLocalSamples();
  ///@}

  /**
   * Return a single sample row by global row.
   *
   * @param row_index The row index of sample value to compute.
   * @return A vector for the given row.
   */
  std::vector<Real> getSampleRow(dof_id_type row_index) const;

  /**
   * Return a single sample value by global row and column index.
   *
   * @param row_index The row index of sample value to compute.
   * @param col_index The column index of sample value to compute.
   * @return The value for the given row and column.
   */
  Real getSample(dof_id_type row_index, dof_id_type col_index) const;

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

  /**
   * Reference to rank configuration defining the partitioning of the sampler matrix
   * This is primarily used by MultiApps to ensure consistent partitioning
   */
  const LocalRankConfig & getRankConfig(bool batch_mode) const
  {
    return batch_mode ? _rank_config.second : _rank_config.first;
  }

  /**
   * Returns true if the adaptive sampling is completed
   */
  virtual bool isAdaptiveSamplingCompleted() const
  {
    mooseError("This method should be overridden in adaptive sampling classes.");
    return false;
  }

  /**
   * Return the parallel communicator
   */
  libMesh::Parallel::Communicator & getLocalComm() { return _local_comm; }

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
   * Get nth random number from the generator.
   * @param n 0-based index of the random number to generate
   * @param index The index of the seed, by default this is zero. To add additional seeds
   *              indices call the setNumberOfRequiedRandomSeeds method.
   *
   * @return A double for the random number, this is double because MooseRandom class uses double.
   */
  Real getRand(std::size_t n, unsigned int index = 0) const;

  /**
   * Get nth random integer from the generator within the specified range [lower, upper)
   * @param n 0-based index of the random number to generate
   * @param lower Lower bounds
   * @param upper Upper bounds
   * @param index The index of the seed, by default this is zero. To add additional seeds
   *              indices call the setNumberOfRequiedRandomSeeds method.
   *
   * @return A integer for the random number
   */
  unsigned int
  getRandl(std::size_t n, unsigned int lower, unsigned int upper, unsigned int index = 0) const;

  /**
   * Base class must override this method to supply the sample distribution data.
   * @param row_index The row index of sample value to compute.
   * @param col_index The column index of sample value to compute.
   * @return The value for the given row and column.
   */
  virtual Real computeSample(dof_id_type row_index, dof_id_type col_index) const = 0;

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

  /**
   * Method to populate a complete row of sample data.
   * @param i The global row index to compute
   * @param data The correctly sized vector of sample value to poplulate

   * This method should not be called directly, it is automatically called by the public
   * getGlobalSamples(), getLocalSamples(), getSampleRow(), and getSample() methods.
   */
  virtual void computeSampleRow(dof_id_type i, std::vector<Real> & data) const;

  /**
   * Method for advancing the random number generator(s) by the supplied number or calls to rand().
   */
  virtual void advanceGenerators(const dof_id_type count);
  virtual void advanceGenerator(const unsigned int seed_index, const dof_id_type count);
  void setAutoAdvanceGenerators(const bool state);

  //@{
  /**
   * Callbacks for before and after execute.
   *
   * These were added to support of dynamic sampler sizes. Recall that execute is simply to advance
   * the state of the generators such that the next sample will be unique. These methods allow
   * operations before and after the call to generator advancement.
   */
  virtual void executeSetUp() {}
  virtual void executeTearDown() {}
  ///@}

  /**
   * This is where the sampler partitioning is defined. It is NOT recommended to
   * override this function unless you know EXACTLY what you are doing
   */
  virtual LocalRankConfig constructRankConfig(bool batch_mode) const;

  /// The minimum number of processors that are associated with a set of rows
  const dof_id_type _min_procs_per_row;
  /// The maximum number of processors that are associated with a set of rows
  const dof_id_type _max_procs_per_row;

  /// Communicator that was split based on samples that have rows
  libMesh::Parallel::Communicator _local_comm;

private:
  ///@{
  /**
   * Functions called by MOOSE to setup the Sampler for use. The primary purpose is to partition
   * the DenseMatrix rows for parallel distribution. A separate methods are required so that the
   * set methods can be called within the constructors of child objects, see
   * FEProblemBase::addSampler method. The reinit was added to support re-partitioning to allow
   * for dynamic changes in sampler size.
   *
   * This init() method is called by FEProblemBase::addSampler; it should not be called elsewhere.
   */
  void init();   // sets up MooseRandom
  void reinit(); // partitions sampler output
  ///@}
  friend void FEProblemBase::addSampler(const std::string & type,
                                        const std::string & name,
                                        InputParameters & parameters);
  /**
   * Advance MooseRandomStateless generators so that new calls to
   * sample methods will create new numbers.
   */
  void execute();
  friend void FEProblemBase::objectExecuteHelper<Sampler>(const std::vector<Sampler *> & objects);

  /**
   * Helper function for reinit() errors.
   **/
  void checkReinitStatus() const;

  /**
   * Advance method for internal use that considers the auto advance flag.
   */
  void advanceGeneratorsInternal(const dof_id_type count);

  /// Random number generators, don't give users access. Control it via the interface from this class.
  std::vector<std::unique_ptr<MooseRandomStateless>> _generators;

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

  /// Flag to indicate if the init method for this class was called
  bool _initialized;

  /// Flag to indicate if the reinit method should be called during execute
  bool _needs_reinit;

  /// Flag for initial execute to allow the first set of random numbers to be always be the same
  bool _has_executed;

  /// Max number of entries for matrix returned by getGlobalSamples
  const dof_id_type _limit_get_global_samples;

  /// Max number of entries for matrix returned by getLocalSamples
  const dof_id_type _limit_get_local_samples;

  /// Max number of entries for matrix returned by getSampleRow
  const dof_id_type _limit_get_row;

  /// The partitioning of the sampler matrix, built in reinit()
  /// first is for normal mode and send is for batch mode
  std::pair<LocalRankConfig, LocalRankConfig> _rank_config;

  /// Flag for disabling automatic generator advancing
  bool _auto_advance_generators;
};
