//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GeneralReporter.h"

class AdaptiveMonteCarloDecision : public GeneralReporter
{
public:
  static InputParameters validParams();
  AdaptiveMonteCarloDecision(const InputParameters & parameters);
  virtual void initialize() override;
  virtual void finalize() override {}
  virtual void execute() override;

protected:

  Real computeMIN(const std::vector<Real> & data);
  std::vector<Real> sortOUTPUT(const std::vector<Real> & outputs, const int & samplessub, const unsigned int & subset, const Real & subset_prob);
  std::vector<Real> sortINPUT(const std::vector<Real> & inputs, const std::vector<Real> & outputs, const int & samplessub, const unsigned int & subset, const Real & subset_prob);

  /// This will add another type of reporter to the params
  template <typename T>
  static InputParameters addReporterTypeParams(const std::string & prefix, bool add_vector = true);

  ///@{
  /// Helper for declaring constant reporter values
  template <typename T>
  std::vector<T *> declareAdaptiveMonteCarloDecisionValues(const std::string & prefix);
  template <typename T>
  std::vector<std::vector<T> *> declareAdaptiveMonteCarloDecisionVectorValues(const std::string & prefix);
  ///@}
  ///@{
  /// Helper for declaring constant reporter values
  template <typename T>
  std::vector<T *> declareAMCSStatistics(const std::string & statistics);
  ///@}

  /// Real reporter data
  std::vector<Real *> _output;
  std::vector<Real *> _inputs;
  std::vector<unsigned int *> _subset_out;

private:
  const int & _step;
  Sampler * _sampler;
  int _ind_sto;
  std::vector<Real> _markov_seed;
  unsigned int _count;
  unsigned int _count_max;
  // std::vector<Real> _output_sorted;
  // std::vector<Real> _outputs_sto;
  // std::vector<std::vector<Real>> _inputs_sto;
  // std::vector<std::vector<Real>> _inputs_sorted;
  std::vector<Real> _output_sorted;
  std::vector<std::vector<Real>> _inputs_sorted;
  std::vector<std::vector<Real>> _inputs_sto;
  std::vector<Real> _outputs_sto;
  unsigned int _subset;
  std::vector<Real> _output_limits;
  int _check_even;
  std::vector<Real> _prev_val;
  Real _prev_val_out;

};

template <typename T>
InputParameters
AdaptiveMonteCarloDecision::addReporterTypeParams(const std::string & prefix, bool add_vector)
{
  InputParameters params = emptyInputParameters();

  params.addParam<std::vector<ReporterValueName>>(prefix + "_names",
                                                  "Names for each " + prefix + " value.");
  params.addParam<std::vector<T>>(prefix + "_values", "Values for " + prefix + "s.");
  if (add_vector)
  {
    params.addParam<std::vector<ReporterValueName>>(
        prefix + "_vector_names", "Names for each vector of " + prefix + "s value.");
    params.addParam<std::vector<std::vector<T>>>(prefix + "_vector_values",
                                                 "Values for vectors of " + prefix + "s.");
  }

  return params;
}

template <typename T>
std::vector<T *>
AdaptiveMonteCarloDecision::declareAdaptiveMonteCarloDecisionValues(const std::string & prefix)
{
  std::string names_param(prefix + "_names");
  std::string values_param(prefix + "_values");
  std::vector<T *> data;

  if (isParamValid(names_param) && !isParamValid(values_param))
    paramError(names_param, "Must specify values using ", values_param);
  else if (!isParamValid(names_param) && isParamValid(values_param))
    paramError(values_param, "Use ", names_param, " to specify reporter names.");
  else if (!isParamValid(names_param) && !isParamValid(values_param))
    return data;

  auto & names = getParam<std::vector<ReporterValueName>>(names_param);
  auto & values = this->getParam<std::vector<T>>(values_param);
  if (names.size() != values.size())
    paramError(values_param,
               "Number of names specified in ",
               names_param,
               " must match number of values specified in ",
               values_param);

   // std::cout << "here " << names[0] << std::endl;
   // std::cout << "here1 " << values[0] << std::endl;
  for (unsigned int i = 0; i < names.size(); ++i)
    data.push_back(&this->declareValueByName<T>(names[i], values[i]));

  return data;
}
