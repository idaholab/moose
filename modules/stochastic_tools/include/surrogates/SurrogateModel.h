//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "StochasticToolsApp.h"
#include "MooseObject.h"
#include "SamplerInterface.h"
#include "SurrogateModelInterface.h"
#include "RestartableModelInterface.h"

class SurrogateModel : public MooseObject,
                       public SamplerInterface,
                       public SurrogateModelInterface,
                       public RestartableModelInterface
{
public:
  static InputParameters validParams();
  SurrogateModel(const InputParameters & parameters);

  static MooseEnum defaultPredictorTypes() { return MooseEnum("real"); }
  static MooseEnum defaultResponseTypes() { return MooseEnum("real vector_real"); }

  /**
   * Evaluate surrogate model given a row of parameters.
   */
  virtual Real evaluate(const std::vector<Real> & x) const
  {
    evaluateError(x, Real());
    return 0.0;
  };

  /// @{
  /**
   * Various evaluate methods that can be overriden
   */
  virtual void evaluate(const std::vector<Real> & x, std::vector<Real> & y) const
  {
    evaluateError(x, y);
  }
  ///@}

  /// @{
  /**
   * Evaluate methods that also return predicted standard deviation (see GaussianProcess.h)
   */
  virtual Real evaluate(const std::vector<Real> & x, Real & std) const
  {
    evaluateError(x, std, true);
    return 0.0;
  }
  virtual void
  evaluate(const std::vector<Real> & x, std::vector<Real> & y, std::vector<Real> & /*std*/) const
  {
    evaluateError(x, y, true);
  }
  ///@}

private:
  template <typename P, typename R>
  void evaluateError(P x, R y, bool with_std = false) const;
};

template <typename P, typename R>
void
SurrogateModel::evaluateError(P /*x*/, R /*y*/, bool with_std) const
{
  std::stringstream ss;
  ss << "Evaluate method";
  if (with_std)
    ss << " (including standard deviation computation)";
  ss << " with predictor type " << MooseUtils::prettyCppType<P>();
  ss << " and response type " << MooseUtils::prettyCppType<R>();
  ss << " has not been implemented.";
  mooseError(ss.str());
}
