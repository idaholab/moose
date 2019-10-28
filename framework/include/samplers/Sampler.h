//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "libmesh/dense_matrix.h"

// MOOSE includes
#include "MooseObject.h"
#include "MooseRandom.h"
#include "SetupInterface.h"
#include "DistributionInterface.h"
#include "Distribution.h"

template <>
InputParameters validParams<Sampler>();

/**
 * This is the base class for Samplers.
 *
 * A sampler is responsible for sampling distributions and providing an API for providing
 * the sample data to objects. The sampler is designed to handle any number of random number
 * generators.
 *
 * The main methods in this object is the getSamples/getLocalSamples methods which return the
 * distribution samples. These methods will return the same results for each call regardless of the
 * number of calls.
 *
 * Samplers support the use of "execute_on", which when called results in new set of random numbers,
 * thus after execute() calls to getSamples/getLocalSamples() methods will now produce a new set of
 * random numbers from calls prior to the execute() call.
 */
class Sampler : public MooseObject, public SetupInterface, public DistributionInterface
{
public:
  static InputParameters validParams();

  Sampler(const InputParameters & parameters);

  ///@{
  /**
   * Return the sampled complete or distributed sample data.
   */
  DenseMatrix<Real> getSamples();
  DenseMatrix<Real> getLocalSamples();
  ///@}

  /**
   * Return the "next" local row. This is designed to be used within a loop using the
   * getLocalRowBegin/End methods as such:
   *
   * for (dof_id_type i = getLocalRowBegin(); i < getLocalRowEnd(); ++i)
   *     std::vector<Real> row = getNextLocalRow();
   *
   * Calls to getNextLocalRow() will continue to return the next row of data until the last local
   * row has been reached, it will then start again at the beginning if called again. Also, calls
   * to getNextLocalRow() can be partial, followed by call(s) to getSamples or getLocalSamples.
   * Continued calls to getNextLocalRow() will still continue to give the next row as if the
   * other get calls were not made. However, when this occurs calls to restore and advance the
   * generators are made after each call got getSamples or getLocalSamples, so this generally
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
  ///@{
  /**
   * These methods should be called within the constructor of child classes to define the size of
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
  double getRand(unsigned int index = 0);

  /**
   * Base class must override this method to supply the sample distribution data.
   * @param row_index The row index of sample value to compute.
   * @param col_index The column index of sample value to compute.
   * @return The value for the given row and column.
   */
  virtual Real computeSample(dof_id_type row_index, dof_id_type col_index) = 0;

  ///@{
  /**
   * Setup method called prior and after looping through distributions.
   *
   * These methods should not be called directly, each is automatically called by the public
   * getSamples() or getLocalSamples() methods.
   */
  virtual void sampleSetUp(){};
  virtual void sampleTearDown(){};
  ///@}

  ///@{
  /**
   * Methods to populate the global or local sample matrix.
   * @param matrix The correctly sized matrix of sample values to populate
   *
   * These methods should not be called directly, each is automatically called by the public
   * getSamples() or getLocalSamples() methods.
   */
  virtual void computeSampleMatrix(DenseMatrix<Real> & matrix);
  virtual void computeLocalSampleMatrix(DenseMatrix<Real> & matrix);
  ///@}

  /**
   * Method for advancing the random number generator(s) by the supplied number or calls to rand().
   *
   * TODO: This should be updated if the If the random number generator is updated to type that
   * supports native advancing.
   */
  virtual void advanceGenerators(dof_id_type count);

  /**
   * Method for manually setting the local row index for iteration of sample rows using
   * getNextLocalRow method.
   */
  void setNextLocalRowIndex(dof_id_type index = 0);

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
  friend void FEProblemBase::addSampler(std::string type,
                                        const std::string & name,
                                        InputParameters & parameters);

  /**
   * Store the state of the MooseRandom generator so that new calls to getSamples/getLocalSamples
   * methods will create new numbers. This also handles sample data cache, if enabled.
   *
   * The execute() method is called in the init() method of this class and
   * FEProblemBase::executeSamplers; it should not be called elsewhere.
   */
  void execute();
  friend void FEProblemBase::executeSamplers(const ExecFlagType & exec_type);

  /// Random number generator, don't give users access. Control it via the interface from this class.
  MooseRandom _generator;

  /// Seed generator
  MooseRandom _seed_generator;

  /// Initial random number seed
  const unsigned int & _seed;

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

  /// Iterator index for getNextLocalRow method
  dof_id_type _next_local_row;

  /// Flag for restoring state during getNextLocalRow iteration
  bool _next_local_row_requires_state_restore;

  /// Flag to indicate if the init method for this class was called
  bool _initialized = false;
};
