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

#ifndef STEPPERINTERFACE_H
#define STEPPERINTERFACE_H

// MOOSE includes
#include "MooseTypes.h"
#include "DependencyResolverInterface.h"

// Standard includes
#include <string>

// Forward Declarations
class FEProblemBase;
class InputParameters;
class StepperName;
class MooseObject;

/**
 * Interface class for classes which interact with Postprocessors.
 * Provides the getPostprocessorValueXYZ() and related interfaces.
 */
class StepperInterface : public DependencyResolverInterface
{
public:
  StepperInterface(const MooseObject * moose_object);

  /**
   * Get the DT from another Stepper.
   *
   * Note: This is intended to be called in the constructor of a Stepper
   * This returns a _reference_ and must be caught as a reference.
   * The value within this reference will automatically be updated by the system.
   *
   * @param name The name of the InputParameter holding the Stepper name to get the value from.
   */
  const Real & getStepperDT(const std::string & name);

  /**
   * Get the DT from another Stepper.
   *
   * Note: This is intended to be called in the constructor of a Stepper
   * This returns a _reference_ and must be caught as a reference.
   * The value within this reference will automatically be updated by the system.
   *
   * @param name The name of the Stepper to get the value from.
   */
  const Real & getStepperDTByName(const StepperName & name);

  /**
   * For DependencyResolverInterface
   */
  const std::set<std::string> & getRequestedItems() override;

  /**
   * For DependencyResolverInterface
   */
  const std::set<std::string> & getSuppliedItems() override;

protected:
  /**
   * Set the name of the supplied item
   */
  void setSuppliedItemName(const StepperName & item_name);

  /**
   * Check to see if a Stepper exists
   *
   * @param name The name of the parameter holding the name of the Stepper
   */
  bool hasStepper(const std::string & name) const;

  /**
   * Check to see if a Stepper exists
   *
   * @param name The name of the Stepper
   */
  bool hasStepperByName(const PostprocessorName & name) const;

private:
  /// The name of this Stepper for the DependencyResolverInterface
  std::set<std::string> _si_name;

  /// The set of Steppers this Stepper depends on
  std::set<std::string> _depend_steppers;

  /// StepperInterface Parameters
  const InputParameters & _si_params;

  /// Reference the the FEProblem class
  FEProblemBase & _si_feproblem_base;

  /// Default value to send back for optional coupling
  Real _default_dt;
};

#endif //STEPPERINTERFACE_H
