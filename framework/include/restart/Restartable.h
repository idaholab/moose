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

class PostprocessorData;
class SubProblem;

/**
 * A  class for creating restricted objects
 * \see BlockRestartable BoundaryRestartable
 */
class Restartable
{
public:
  /**
   * Class constructor
   * Populates the SubProblem and MooseMesh pointers
   *
   * @param name The name of the object
   * @param parameters The InputParameters for the object.
   * @param system_name The name of the MOOSE system.  ie "Kernel", "BCs", etc.  Should roughly correspond to the section in the input file so errors are easy to understand.
   */
  Restartable(std::string name, InputParameters & parameters, std::string system_name);

  /**
   * Constructor for objects that don't have "parameters"
   *
   * @param name The name of the object
   * @param system_name The name of the MOOSE system.  ie "Kernel", "BCs", etc.  Should roughly correspond to the section in the input file so errors are easy to understand.
   * @param fe_problem A reference to the fe_problem for this object
   * @param tid Optional thread id (will default to zero)
   */
  Restartable(std::string name, std::string system_name, SubProblem & subproblem, THREAD_ID tid = 0);

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
   *
   * @param data_name The name of the data (usually just use the same name as the member variable)
   * @param context Optional context pointer that will be passed to the load and store functions
   */
  template<typename T>
  T & declareRestartableData(std::string data_name, void * context = NULL);

  /**
   * Declare a piece of data as "restartable" and initialize it.
   * This means that in the event of a restart this piece of data
   * will be restored back to its previous value.
   *
   * NOTE: This returns a _reference_!  Make sure you store it in a _reference_!
   *
   * @param data_name The name of the data (usually just use the same name as the member variable)
   * @param init_value The initial value of the data
   * @param context Optional context pointer that will be passed to the load and store functions
   */
  template<typename T>
  T & declareRestartableData(std::string data_name, const T & init_value, void * context = NULL);

private:
  /**
   * Note: This is only used internally in MOOSE.  DO NOT use this function!
   *
   * Declare a piece of data as "restartable".
   * This means that in the event of a restart this piece of data
   * will be restored back to its previous value.
   *
   * NOTE: This returns a _reference_!  Make sure you store it in a _reference_!
   *
   * @param data_name The name of the data (usually just use the same name as the member variable)
   * @param object_name A supplied name for the object that is declaring this data.
   */
  template<typename T>
  T & declareRestartableDataWithObjectName(std::string data_name, std::string object_name, void * context = NULL);

  /// Helper function so we don't have to #include SubProblem in the header
  void registerRestartableDataOnSubProblem(std::string name, RestartableDataValue * data, THREAD_ID tid);

  /// The name of the object
  std::string _restartable_name;

  /// The object's parameters
  InputParameters * _restartable_params;

  /// The system name this object is in
  std::string _restartable_system_name;

  /// The thread ID for this object
  THREAD_ID _restartable_tid;

  /// Pointer to the SubProblem class
  SubProblem * _restartable_subproblem;

  /// For access to registerRestartableDataOnSubProblem()
  friend class PostprocessorData;
  friend class NearestNodeLocator;
};

template<typename T>
T &
Restartable::declareRestartableData(std::string data_name, void * context)
{
  if(!_restartable_subproblem)
    mooseError("No valid SubProblem found for " << _restartable_system_name << "/" << _restartable_name);

  std::string full_name = _restartable_system_name + "/" + _restartable_name + "/" + data_name;
  RestartableData<T> * data_ptr = new RestartableData<T>(full_name, context);

  registerRestartableDataOnSubProblem(full_name, data_ptr, _restartable_tid);

  return data_ptr->get();
}

template<typename T>
T &
Restartable::declareRestartableData(std::string data_name, const T & init_value, void * context)
{
  if(!_restartable_subproblem)
    mooseError("No valid SubProblem found for " << _restartable_system_name << "/" << _restartable_name);

  std::string full_name = _restartable_system_name + "/" + _restartable_name + "/" + data_name;
  RestartableData<T> * data_ptr = new RestartableData<T>(full_name, context);

  data_ptr->set() = init_value;

  registerRestartableDataOnSubProblem(full_name, data_ptr, _restartable_tid);

  return data_ptr->get();
}

template<typename T>
T &
Restartable::declareRestartableDataWithObjectName(std::string data_name, std::string object_name, void * context)
{
  std::string old_name = _restartable_name;

  _restartable_name = object_name;

  T & value = declareRestartableData<T>(data_name, context);

  _restartable_name = old_name;

  return value;
}

#endif // RESTARTABLE
