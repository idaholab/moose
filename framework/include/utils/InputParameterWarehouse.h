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

#ifndef INPUTPARAMETERWAREHOUSE_H
#define INPUTPARAMETERWAREHOUSE_H

// MOOSE includes
#include "ParallelUniqueId.h"
#include "MooseObjectName.h"
#include "MooseTypes.h"
#include "ControllableParameter.h"

// Forward declarations
class InputParameters;

/**
 * Storage container for all InputParamter objects.
 *
 * This object is responsible for InputParameter objects, all MooseObjects should
 * contain a reference to the parameters object stored here.
 *
 */
class InputParameterWarehouse
{
public:
  /**
   * Class constructor
   */
  InputParameterWarehouse();

  /**
   * Destruction
   */
  virtual ~InputParameterWarehouse() = default;

  ///@{
  /**
   * Return a const reference to the InputParameters for the named object
   * @param tag The tag of the object (e.g., 'Kernel')
   * @param name The name of the parameters object, including the tag (name only input) or
   * MooseObjectName object
   * @param tid The thread id
   * @return A const reference to the warehouse copy of the InputParameters
   */
  const InputParameters & getInputParametersObject(const std::string & name,
                                                   THREAD_ID tid = 0) const;
  const InputParameters & getInputParametersObject(const std::string & tag,
                                                   const std::string & name,
                                                   THREAD_ID tid = 0) const;
  const InputParameters & getInputParametersObject(const MooseObjectName & object_name,
                                                   THREAD_ID tid = 0) const;
  ///@{
  /**
   * Return const reference to the map containing the InputParameter objects
   */
  const std::multimap<MooseObjectName, std::shared_ptr<InputParameters>> &
  getInputParameters(THREAD_ID tid = 0) const;

  /**
   * Returns a ControllableParameter object
   * @see Control
   */
  template <typename T>
  ControllableParameter<T> getControllableParameter(const MooseObjectParameterName & desired,
                                                    bool mark_as_controlled = false);

  /**
   * Method for linking control parameters of different names
   */
  void addControllableParameterConnection(const MooseObjectParameterName & master,
                                          const MooseObjectParameterName & slave);

private:
  /// Storage for the InputParameters objects
  std::vector<std::multimap<MooseObjectName, std::shared_ptr<InputParameters>>> _input_parameters;

  /// InputParameter links
  std::map<MooseObjectParameterName, std::vector<MooseObjectParameterName>> _input_parameter_links;

  /// A list of parameters that were controlled (only used for output)
  std::map<std::shared_ptr<InputParameters>, std::set<MooseObjectParameterName>>
      _controlled_parameters;

  /**
   * Method for adding a new InputParameters object
   * @param parameters The InputParameters object to copy and store in the warehouse
   * @return A reference to the warehouse copy of the InputParameters, this
   *         is what should be passed into the MooseObjects constructors.
   *
   * A new object is created from the old object because InputParameters objects
   * are generic until Factory::create() is called and the actual MooseObject
   * is created.
   *
   * This method is private, because only the factories that are creating objects should be
   * able to call this method.
   */
  InputParameters &
  addInputParameters(const std::string & name, InputParameters parameters, THREAD_ID tid = 0);

  ///@{
  /**
   * Return a reference to the InputParameters for the named object
   * @param tag The tag of the object (e.g., 'Kernel')
   * @param name The name of the parameters object, including the tag (name only input) or
   * MooseObjectName object
   * @param tid The thread id
   * @return A reference to the warehouse copy of the InputParameters
   *
   * If you are using this method to access a writable reference to input parameters, this
   * will break the ability to control the parameters with the MOOSE control logic system.
   * Only change parameters if you know what you are doing. Hence, this is private for a reason.
   */
  InputParameters & getInputParameters(const std::string & name, THREAD_ID tid = 0) const;
  InputParameters &
  getInputParameters(const std::string & tag, const std::string & name, THREAD_ID tid = 0) const;
  InputParameters & getInputParameters(const MooseObjectName & object_name,
                                       THREAD_ID tid = 0) const;
  ///@{

  /**
   * Return the list of controlled parameters (used for output)
   * @see ControlOutput
   */
  const std::map<std::shared_ptr<InputParameters>, std::set<MooseObjectParameterName>> &
  getControlledParameters()
  {
    return _controlled_parameters;
  }
  void clearControlledParameters() { _controlled_parameters.clear(); }

  friend class Factory;
  friend class ActionFactory;
  friend class Control;
  friend class ControlOutput;

  // RELAP-7 Control Logic (This will go away when the MOOSE system is created)
  friend class Component;
};

template <typename T>
ControllableParameter<T>
InputParameterWarehouse::getControllableParameter(const MooseObjectParameterName & input,
                                                  bool mark_as_controlled /*=false*/)
{

  // The ControllableParameter object to return
  ControllableParameter<T> output;

  // Vector of desired parameters
  std::vector<MooseObjectParameterName> params(1, input);
  const auto link_it = _input_parameter_links.find(input);
  if (link_it != _input_parameter_links.end())
    params.insert(params.end(), link_it->second.begin(), link_it->second.end());

  // Loop over all threads
  for (THREAD_ID tid = 0; tid < libMesh::n_threads(); ++tid)
  {
    // Loop over all InputParameter objects
    for (const auto & param_pair : _input_parameters[tid])
    {
      // Loop of all desired params
      for (const auto & param : params)
      {
        // If the desired object name does not match the current object name, move on
        MooseObjectParameterName desired = param;

        if (desired != param_pair.first)
          continue;

        // If the parameter is valid and controllable update the output vector with a pointer to the
        // parameter
        if (param_pair.second->libMesh::Parameters::have_parameter<T>(desired.parameter()))
        {
          // Do not allow non-controllable types to be controlled
          if (!param_pair.second->isControllable(desired.parameter()))
            mooseError("The desired parameter is not controllable: ", desired);

          // Store pointer to the writable parameter
          output.insert(desired, param_pair.second);

          if (mark_as_controlled && tid == 0)
            _controlled_parameters[param_pair.second].insert(desired);
        }
      }
    }
  }

  // Error if nothing was found
  if (output.size() == 0)
    mooseError("The controlled parameter was not found: ", input);

  return output;
}

#endif // INPUTPARAMETERWAREHOUSE_H
