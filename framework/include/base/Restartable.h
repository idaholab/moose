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

#ifndef RESTARTABLE_H
#define RESTARTABLE_H

#include "InputParameters.h"
#include "MooseTypes.h"
#include "RestartableData.h"
#include "ParallelUniqueId.h"

/**
 * A  class for creating restricted objects
 * \see BlockRestartable BoundaryRestartable
 */
class Restartable
{
public:
  /**
   * Class constructor
   * Populates the FEProblem and MooseMesh pointers
   *
   * @param name The name of the object
   * @param parameters The InputParameters for the object.
   * @param system_name The name of the MOOSE system.  ie "Kernel", "BCs", etc.  Should roughly correspond to the section in the input file so errors are easy to understand.
   */
  Restartable(std::string name, InputParameters & parameters, std::string system_name);

  /**
   * Emtpy destructor
   */
  virtual ~Restartable();

protected:

  /**
   * Declare a piece of data as "restartable".
   * This means that in the event of a restart this piece of data
   * will be restored back to its previous value.
   *
   * NOTE: This returns a _reference_!  Make sure you store it in a _reference_!
   */
  template<typename T>
  T & declareRestartableData(std::string data_name);

private:
  /// Helper function so we don't have to #include FEProblem in the header
  void registerRestartableDataOnFEProblem(std::string name, RestartableDataValue * data, THREAD_ID tid);

  /// The name of the object
  std::string _restartable_name;

  /// The object's parameters
  InputParameters & _restartable_params;

  /// The system name this object is in
  std::string _restartable_system_name;

  /// The thread ID for this object
  THREAD_ID _restartable_tid;

  /// Pointer to the FEProblem class
  FEProblem * _restartable_feproblem;
};

template<typename T>
T &
Restartable::declareRestartableData(std::string data_name)
{
  std::string full_name = _restartable_system_name + "/" + _restartable_name + "/" + data_name;
  RestartableData<T> * data_ptr = new RestartableData<T>(full_name);

  registerRestartableDataOnFEProblem(full_name, data_ptr, _restartable_tid);

  return data_ptr->get();
}

#endif // RESTARTABLE
