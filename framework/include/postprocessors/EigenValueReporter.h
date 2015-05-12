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

#ifndef EIGENVALUEREPORTER_H
#define EIGENVALUEREPORTER_H

// MOOSE includes
#include "GeneralPostprocessor.h"

// Forward declarations
class EigenValueReporter;
class EigenExecutionerBase;

template<>
InputParameters validParams<EigenValueReporter>();

/**
 * A class to report the Eigen value from the EigenExecutionerBase
 * object.
 */
class EigenValueReporter : public GeneralPostprocessor
{
public:

  /**
   * Class constructor
   * @param parameters
   */
  EigenValueReporter(const std::string & name, InputParameters parameters);

  /**
   * Class destructor
   */
  virtual ~EigenValueReporter(){}

  /**
   * Populates the pointer to the executioner that holds the eigen value
   */
  virtual void initialSetup();

  ///@{
  /**
   * Un-used methods required to be defined
   */
  virtual void execute(){}
  virtual void initialize(){}
  virtual void finalize(){}
  ///@}

  /**
   * Returns the value of the eigen value as computed
   * by an EigenExecutionerBase object.
   */
  virtual PostprocessorValue getValue();

private:

  /// Pointer to the EigenExecutionerBase where the eigen value is stored
  EigenExecutionerBase * _eigen_executioner;
};

#endif //EIGENVALUEREPORTER_H
