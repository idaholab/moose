/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#ifndef SAMPLER_H
#define SAMPLER_H

// libMesh includes
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
   * Store the state of the MooseRandom generator so that new calls to getSamples will create
   * new numbers.
   */
  void execute();

protected:
  /**
   * Get the next random number from the generator.
   * @param offset The index of the seed, by default this is zero. To add additional seeds
   *               indices call the setNumberOfRequiedRandomSeeds method.
   *
   * Returns a double for the random number, this is double because MooseRandom class uses double.
   */
  double rand(unsigned int index = 0);

  /**
   * Base class must override this method to supply the sample distribution data.
   */
  virtual DenseMatrix<Real> sampleDistribution(Distribution &, unsigned int) = 0;

  /**
   * Set the number of seeds required by the sampler. The Sampler will generate
   * additional seeds as needed. This function should be called in the constructor
   * of child objects.
   */
  void setNumberOfRequiedRandomSeeds(const std::size_t & number);

  /// Map used to store the perturbed parameters and their corresponding distributions
  std::vector<Distribution *> _distributions;

  /// Distribution names
  const std::vector<DistributionName> & _distribution_names;

private:
  /// Random number generator, don't give users access we want to control it via the interface
  /// from this class.
  MooseRandom _generator;

  /// Initial random number seed
  const unsigned int & _seed;

  /// Generated list of seeds
  std::vector<unsigned int> _seeds;
};

#endif /* SAMPLER_H */
