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

#ifndef RELATIVESOLUTIONNORM_H
#define RELATIVESOLUTIONNORM_H

#include "GeneralPostprocessor.h"

// Forward Declarations
class RelativeSolutionDifferenceNorm;
class Transient;

template <>
InputParameters validParams<RelativeSolutionDifferenceNorm>();

/**
 * Gets the relative solution norm from the transient executioner
 */
class RelativeSolutionDifferenceNorm : public GeneralPostprocessor
{
public:
  /**
   * Class constructor
   * @param parameters The input parameters
   */
  RelativeSolutionDifferenceNorm(const InputParameters & parameters);

  ///@{
  /**
   * No action taken
   */
  virtual void initialize() override {}
  virtual void execute() override {}
  ///@}

  /**
   * Returns the relative solution norm taken from the transient executioner
   * @return A const reference to the value of the postprocessor
   */
  virtual Real getValue() override;

protected:
  /// Transient executioner
  Transient * _trex;
};

#endif // RELATIVESOLUTIONNORM_H
