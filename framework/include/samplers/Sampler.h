//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef SAMPLER_H
#define SAMPLER_H

#include "libmesh/dense_matrix.h"

// MOOSE includes
#include "MooseObject.h"
#include "MooseRandom.h"
#include "SetupInterface.h"
#include "DistributionInterface.h"
#include "Distribution.h"

class Sampler;

template <>
InputParameters validParams<Sampler>();

/**
 * This is the base class for Samplers.
 *
 * A sampler is responsible for sampling distributions and providing an interface for providing
 * the samples to objects. The sampler is designed to handle any number of random number generators.
 *
 * The main method in this object is the getSamples method which returns the distribution samples.
 * This method will return the same results for each call when the "_generator" from this class
 * is used by child classes.
 *
 * Samplers support the use of "execute_on", which when called results in new set of random numbers,
 * thus after execute() runs the getSamples() method will now produces a new set of random numbers
 * from calls prior to the execute() call.
 */
class Sampler : public MooseObject, public SetupInterface, public DistributionInterface
{
public:
  Sampler(const InputParameters & parameters);

  /**
   * Simple object for storing the sampler location (see SamplerMultiApp).
   * @param s Sample index (index associated with DenseMatrix returned by Sampler::getSamples())
   * @param r Row index (index associated with the row in the DenseMatrix defined by s).
   */
  struct Location
  {
    Location(const unsigned int & s, const unsigned int & r) : _sample(s), _row(r) {}

    ///@{
    /**
     * Accessors for the sample and row numbers.
     */
    unsigned int sample() const { return _sample; }
    unsigned int row() const { return _row; }
    ///@}

  private:
    /// Sample number (i.e., the index in the matrix)
    const unsigned int _sample;

    /// Row number for the given sample matrix
    const unsigned int _row;
  };

  /**
   * Return the list of distribution names.
   */
  const std::vector<DistributionName> & getDistributionNames() const { return _distribution_names; }

  ///@{
  /**
   * Setup method called prior and after looping through distributions.
   */
  virtual void sampleSetUp(){};
  virtual void sampleTearDown(){};
  ///@}

  /**
   * Return the sampled distribution data.
   */
  std::vector<DenseMatrix<Real>> getSamples();

  /**
   * Return the sample names, by default 'sample_0, sample_1, etc.' is used.
   * @return The names assigned to the DenseMatrix items returned by getSamples().
   *
   * If custom names are required then utilize the setSampleNames method.
   */
  const std::vector<std::string> & getSampleNames() const { return _sample_names; }

  /**
   * Set the sample names.
   * @param names List of names to assign to the DenseMatrix entries returned by
   *              getSamples() method.
   */
  void setSampleNames(const std::vector<std::string> & names);

  /**
   * Store the state of the MooseRandom generator so that new calls to getSamples will create
   * new numbers.
   */
  void execute();

  /**
   * Return the Sample::Location for the given multi app index.
   * @param global_index The global row, which is the row if all the DenseMatrix objects
   *                     were stacked in order in a single object. When using SamplerMultiApp
   *                     the global_index is the MultiApp global index.
   * @return The location which includes the DenseMatrix index and the row within that matrix.
   */
  Sampler::Location getLocation(unsigned int global_index);

  /**
   * Return the number of samples.
   * @return The total number of rows that exist in all DenseMatrix values from getSamples()
   */
  unsigned int getTotalNumberOfRows();

protected:
  /**
   * Get the next random number from the generator.
   * @param offset The index of the seed, by default this is zero. To add additional seeds
   *               indices call the setNumberOfRequiedRandomSeeds method.
   *
   * @return A double for the random number, this is double because MooseRandom class uses double.
   */
  double rand(unsigned int index = 0);

  /**
   * Base class must override this method to supply the sample distribution data.
   *
   * @return The list of samples for the Sampler.
   */
  virtual std::vector<DenseMatrix<Real>> sample() = 0;

  /**
   * Set the number of seeds required by the sampler. The Sampler will generate
   * additional seeds as needed. This function should be called in the constructor
   * of child objects.
   * @param number The required number of random seeds, by default this is called with 1.
   */
  void setNumberOfRequiedRandomSeeds(const std::size_t & number);

  /**
   * Reinitialize the offsets and row counts.
   * @param data Samples, as returned from getSamples(), to use for re-computing size information
   */
  void reinit(const std::vector<DenseMatrix<Real>> & data);

  /// Map used to store the perturbed parameters and their corresponding distributions
  std::vector<Distribution *> _distributions;

  /// Distribution names
  const std::vector<DistributionName> & _distribution_names;

  /// Sample names
  std::vector<std::string> _sample_names;

private:
  /// Random number generator, don't give users access we want to control it via the interface
  /// from this class.
  MooseRandom _generator;

  /// Seed generator
  MooseRandom _seed_generator;

  /// Initial random number seed
  const unsigned int & _seed;

  /// Data offsets for computing location based on global index
  std::vector<unsigned int> _offsets;

  /// Total number of rows
  unsigned int _total_rows;
};

#endif /* SAMPLER_H */
