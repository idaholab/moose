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

#ifndef REPORTABLEDATA_H
#define REPORTABLEDATA_H

//MOOSE includes
#include "MooseTypes.h"
#include "Restartable.h"

#include <map>

class FEProblem;

/**
 * A storage facility for ReportableValues
 * \see Reportable
 */
class ReportableData : public Restartable
{
public:

  /// A local short-hand for the threaded storage of Reportable value names
  typedef std::vector<std::set<std::string> > ReportableNames;

  /**
   * Class constructor
   * @param fe_problem A reference to FEProblem
   */
  ReportableData(FEProblem & fe_problem);

  /**
   * Initialize a new ReportableValue
   * @param name The name of the new ReportableValue
   * @param value The value to initilize both the current and old value (defaults to 0.0)
   * @param tid The thread id (only used for name checking)
   */
  void init(const std::string & name, Real value=0.0, THREAD_ID tid=0);

  /**
   * Test if a ReportableValue already exists
   * @param name The name to test
   * @param tid The thread id to test on (defaults to 0)
   */
  bool hasReportableValue(const std::string & name, THREAD_ID tid=0);

  /**
   * Get a reference to the ReportableValue
   * @param name The name of the ReportableValue of interest
   * @return Reference to the ReportableValue
   */
  ReportableValue & getReportableValue(const std::string & name);

  /**
   * Get a reference to the ReportableValue from the previous time step
   * @param name The name of the ReportableValue of interest
   * @return Reference to the old ReportableValue
   */
  ReportableValue & getReportableValueOld(const std::string & name);

  /**
   * Store a value as a ReportableValue
   * @param name Name of the value to store
   * @param value The value to store
   */
  void storeValue(const std::string & name, Real value);

  /**
   * Extract a reference to a map of all the reportable values
   * @return Reference to std::map of reportable values
   */
  const std::map<std::string, ReportableValue> & values() const;

  /**
   * Copy the current values to the old values
   */
  void copyValuesBack();

protected:

  /// Map of stored names
  ReportableNames & _names;

  /// Values of the post-processor at the current time
  std::map<std::string, ReportableValue> & _values;

  /// Values of the post-processors at the time t-1
  std::map<std::string, ReportableValue> & _values_old;

};

#endif //REPORTABLEDATA_H
