//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GeneralUserObject.h"
#include "SamplerInterface.h"

// Forward Declarations
class SurrogateModel;

template <>
InputParameters validParams<SurrogateModel>();

class SurrogateModel : public GeneralUserObject, public SamplerInterface
{
public:
  static InputParameters validParams();

  SurrogateModel(const InputParameters & parameters);

  /**
   * Need initial setup since samplers are built after user objects
   */
  virtual void initialSetup() override;

  virtual void initialize() override {}
  virtual void execute() override{};
  virtual void finalize() override{};

  /// Access number of dimensions/parameters
  unsigned int getNumberOfParameters() const { return _ndim; }

  /**
   * Evaluate surrogate model given a row of parameters.
   * This needs to be overridden in derived classes.
   */
  virtual Real evaluate(const std::vector<Real> & x) const = 0;

protected:
  /// Sampler from which the parameters were perturbed
  Sampler * _sampler;
  /// Vector postprocessor of the results from peturbing the model with _sampler
  const VectorPostprocessorValue & _values;
  /// Total number of parameters/dimensions
  unsigned int _ndim;
};
