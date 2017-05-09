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

#include "MooseObject.h"
#include "Distribution.h"
#include "Restartable.h"
#include "DistributionInterface.h"
#include "RandomInterface.h"

class Sampler;

template <>
InputParameters validParams<Sampler>();

/**
 * This is the base class for Samplers and all Samplers should inherit from this class
 */
class Sampler : public MooseObject,
                public RandomInterface,
                public DistributionInterface,
                public Restartable
{
public:
  Sampler(const InputParameters & parameters);
  /**
   * Draw the values from given distributions for all the perturbed parameters
   */
  virtual void generateSamples();
  /**
   * Return a vector of variable names
   */
  virtual std::vector<std::string> getSampledVariableNames();
  /**
   * Return a vector of current sampled values of given parameters
   */
  virtual std::vector<Real> getSampledValues(const std::vector<std::string> & variableNames);
  /**
   * Return the current sampled value of given parameter
   */
  virtual Real getSampledValue(const std::string & variableName);
  /**
   * Return the probability weights for all samples
   */
  virtual std::vector<Real> getProbabilityWeights();
  /**
   * Return true if there were failed runs
   */
  virtual bool checkRuns();

protected:
  THREAD_ID _tid;
  /// control whether the user wants to reseed the distributions at each new sample or not
  const bool _reseed_for_new_sample;
  /// vector of distributions names
  const std::vector<DistributionName> & _dist_names;
  /// vector of perturbed parameters names
  const std::vector<std::string> & _var_names;
  /// counter used to record the index of current samples
  unsigned int _current_sample;
  /// map used to store the perturbed parameters names and values of current samples
  std::map<std::string, Real> _var_value_map;
  /// map used to store the perturbed parameters names and values of all samples
  std::map<std::string, std::vector<Real>> _var_value_hist;
  /// map used to store the perturbed paramters and their corresponding distributions
  std::map<std::string, Distribution *> _var_dist_map;
  /// vector used to store the probability weights of all set of samples
  std::vector<Real> _probability_weight;
  bool _failed_runs;
};

#endif /* SAMPLER_H */
