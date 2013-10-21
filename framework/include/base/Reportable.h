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
#include "ReportableData.h"

// Forward decleration
class FEProblem;

/**
 * An interface for declaring reportable data.
 * By design all data declared as Reportable is also Restartable
 * @see Restartable
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
   * Initilizes a value as being reportable (prefered method)
   * @param name The name of the reportable value (must be unique to this object)
   * @param value The initial value for the data
   * @param output A flag for toggling output (false disables all ouput of this parameter)
   * @return A reference to the ReportableValue
   */
  ReportableValue & declareReportableValue(std::string name, Real value = 0.0, bool output = true);

  /**
   * Initilizes a value as being reportable without automatically adding the object name (advanced method)
   * @param name The name of the reportable value (must be unique to all objects)
   * @param value The initial value for the data
   * @param output A flag for toggling output (false disables all ouput of this parameter)
   * @return A reference to the ReportableValue
   */
  ReportableValue & declareReportableValueByName(std::string name, Real value = 0.0, bool output = true);

  /**
   * Get a constant reference to the reportable value (prefered method)
   * @return A const reference to the ReportableValue
   * @see declareReportableValue
   */
  const ReportableValue & getReportableValue(std::string name);

  /**
   * Get a constant reference to the reportable value withou adding the object name (advanced method)
   * @return A const reference to the ReportableValue
   * @see declareReportableValue
   */
  const ReportableValue & getReportableValueByName(std::string name);

  /**
   * Test if a reportable value exists (prefered method)
   * @return True if the name given is a reportable value
   */
  bool hasReportableValue(std::string name);

  /**
   * Test if a reportable value exists using the exact name supplied (advanced method)
   * @return True if the name given is a reportable value
   */
  bool hasReportableValueByName(std::string name);


protected:

  /**
   * Creates the full name of the reportable value by adding object name as a prefix
   * @param name The short name supplied by the object
   * @return The full long name that contains the object name (e.g., <object>/<value>)
   */
  std::string longName(std::string & name);

  /// Pointer to FEProblem
  FEProblem * _reportable_feproblem;

  /// Reference to the ReportableData class that exists on FEProblem
  ReportableData & _reportable_data;

  /// The thread id for the object
  THREAD_ID _reportable_tid;

  /// The name of the objet
  std::string _reportable_object_name;

};

#endif // REPORTABLE_H
