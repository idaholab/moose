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

#ifndef REPORTABLE_H
#define REPORTABLE_H

#include "InputParameters.h"
#include "FEProblem.h"

/**
 * An interface for declaring reportable data
 *
 */
class Reportable
{
public:

  /**
   * Class constructor
   * @param name The name of the object
   * @param parameters The input parameters
   */
  Reportable(std::string name, InputParameters & parameters);

  /**
   * Virtual desctructor
   */
  virtual ~Reportable();

  /**
   * Initilizes a value as being reportable
   * @return A reference to the ReportableValue
   */
  ReportableValue & declareReportableValue(std::string name, Real value = 0.0);

  /**
   * Get a reference to the reportable value
   * @return A reference to the ReportableValue
   */
  ReportableValue & getReportableValue(std::string name);

  /**
   * Test if a reportable value exists
   * @return True if the name given is a reportable value
   */
  bool hasReportableValue(std::string name);

protected:

  /// Pointer to FEProblem
  FEProblem * _reportable_feproblem;

  /// Reference to the ReportableData class that exists on FEProblem
  ReportableData & _reportable_data;

  /// The thread id for the object
  THREAD_ID _reportable_tid;
};

#endif // REPORTABLE_H
