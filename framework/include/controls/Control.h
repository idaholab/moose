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

#ifndef CONTROL_H
#define CONTROL_H

// MOOSE includes
#include "MooseObject.h"
#include "ControllableParameter.h"
#include "InputParameterWarehouse.h"
#include "TransientInterface.h"
#include "SetupInterface.h"
#include "FunctionInterface.h"
#include "UserObjectInterface.h"
#include "PostprocessorInterface.h"
#include "VectorPostprocessorInterface.h"

// Forward declarations
class Control;
class FEProblemBase;

template <>
InputParameters validParams<Control>();

/**
 * Base class for Control objects.
 *
 * These objects are create by the [Controls] block in the input file after
 * all other MooseObjects are created, so they have access to parameters
 * in all other MooseObjects.
 */
class Control : public MooseObject,
                public TransientInterface,
                public SetupInterface,
                public FunctionInterface,
                public UserObjectInterface,
                protected PostprocessorInterface,
                protected VectorPostprocessorInterface
{
public:
  /**
   * Class constructor
   * @param parameters The input parameters for this control object
   */
  Control(const InputParameters & parameters);

  /**
   * Class destructor
   */
  virtual ~Control() {}

  /**
   * Execute the control. This must be overridden.
   */
  virtual void execute() = 0;

  /**
   * (DEPRECATED) Return the valid "execute_on" options for Control objects
   */
  static MultiMooseEnum getExecuteOptions();

protected:
  /// Reference to the FEProblemBase for this object
  FEProblemBase & _fe_problem;

  ///@{
  /**
   * Obtain the value of a controllable parameter given input file syntax or actual name.
   * @param name The name of the parameter to retrieve a value.
   * @param warn_when_values_diff When true, produce a warning if multiple controllable values share
   *                              the given name but have varying values.
   * @return A constant reference to the first parameter that matches the given name.
   */
  template <typename T>
  const T & getControllableValue(const std::string & name, bool warn_when_values_differ = true);

  template <typename T>
  const T & getControllableValueByName(const std::string & name,
                                       bool warn_when_values_differ = true);

  template <typename T>
  const T & getControllableValueByName(const std::string & tag,
                                       const std::string & param_name,
                                       bool warn_when_values_differ = true);
  ///@}

  ///@{
  /**
   * Set the value(s) of a controllable parameter of class given input file syntax or actual name.
   * @param name The name of the parameter to retrieve a value.
   * @param value The value to set all matching parameters to.
   * @param warn_when_values_diff When true, produce a warning if multiple controllable values share
   * the given
   *                              name but have varying values.
   */
  template <typename T>
  void setControllableValue(const std::string & name,
                            const T & value,
                            bool warn_when_values_differ = false);

  template <typename T>
  void setControllableValueByName(const std::string & name,
                                  const T & value,
                                  bool warn_when_values_differ = false);

  template <typename T>
  void setControllableValueByName(const std::string & tag,
                                  const std::string & param_name,
                                  const T & value,
                                  bool warn_when_values_differ = false);
  ///@}

private:
  /// A reference to the InputParameterWarehouse which is used for access the parameter objects
  InputParameterWarehouse & _input_parameter_warehouse;

  /// Helper method for retrieving controllable parameters
  template <typename T>
  ControllableParameter<T> getControllableParameterHelper(const MooseObjectParameterName & desired,
                                                          bool warn_when_values_differ,
                                                          bool mark_as_set = false);
};

template <typename T>
const T &
Control::getControllableValue(const std::string & name, bool warn_when_values_differ)
{
  return getControllableValueByName<T>(getParam<std::string>(name), warn_when_values_differ);
}

template <typename T>
const T &
Control::getControllableValueByName(const std::string & name, bool warn_when_values_differ)
{
  MooseObjectParameterName desired(name);
  ControllableParameter<T> helper =
      getControllableParameterHelper<T>(desired, warn_when_values_differ);
  return *(helper.get()[0]);
}

template <typename T>
const T &
Control::getControllableValueByName(const std::string & tag,
                                    const std::string & param_name,
                                    bool warn_when_values_differ)
{
  MooseObjectParameterName desired(tag, param_name);
  ControllableParameter<T> helper =
      getControllableParameterHelper<T>(desired, warn_when_values_differ);
  return *(helper.get()[0]);
}

template <typename T>
void
Control::setControllableValue(const std::string & name,
                              const T & value,
                              bool warn_when_values_differ)
{
  setControllableValueByName<T>(getParam<std::string>(name), value, warn_when_values_differ);
}

template <typename T>
void
Control::setControllableValueByName(const std::string & name,
                                    const T & value,
                                    bool warn_when_values_differ)
{
  MooseObjectParameterName desired(name);
  ControllableParameter<T> helper =
      getControllableParameterHelper<T>(desired, warn_when_values_differ, /*mark_as_set=*/true);
  helper.set(value);
}

template <typename T>
void
Control::setControllableValueByName(const std::string & tag,
                                    const std::string & param_name,
                                    const T & value,
                                    bool warn_when_values_differ)
{
  MooseObjectParameterName desired(tag, param_name);
  ControllableParameter<T> helper =
      getControllableParameterHelper<T>(desired, warn_when_values_differ, /*mark_as_set=*/true);
  helper.set(value);
}

template <typename T>
ControllableParameter<T>
Control::getControllableParameterHelper(const MooseObjectParameterName & desired,
                                        bool warn_when_values_differ,
                                        bool mark_as_set)
{

  // The ControllableParameter object to return
  ControllableParameter<T> output =
      _input_parameter_warehouse.getControllableParameter<T>(desired, mark_as_set);

  // Produce a warning, if the flag is true, when multiple parameters have differing values
  if (warn_when_values_differ)
  {
    // Inspect the values for differing values
    const std::vector<T *> & values = output.get();

    // The first parameter to test against
    const T & value0 = *values[0];

    // Loop over all other parameter values
    for (typename std::vector<T *>::const_iterator it = values.begin() + 1; it != values.end();
         ++it)
    {
      if (value0 != **it)
      {
        std::ostringstream oss;
        oss << "The controlled parameter with tag, '" << desired.tag() << "', and name, '"
            << desired.name() << "', in " << name() << " was found,\n";
        oss << "but the number of parameters have differing values.\n\n";
        oss << output.dump();
        mooseWarning(oss.str());
      }
    }
  }

  return output;
}

#endif // CONTROL_H
