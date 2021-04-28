//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html
#pragma once

#include "MooseTypes.h"
#include "ReporterData.h"
#include "MooseObject.h"

// Forward declarations
class FEProblemBase;
class InputParameters;

/**
 * Interface to allow object to consume Reporter values.
 */
class ReporterInterface
{
public:
  static InputParameters validParams();
  ReporterInterface(const MooseObject * moose_object);

protected:
  ///@{
  /**
   * doco-normal-methods-begin
   * Returns read-only reference to a Reporter value that is provided by an input parameter.
   * @tparam T The C++ type of the Reporter value being consumed
   * @param param_name The name of the parameter that gives the name of the Reporter, which
   *                   must be a ReporterName parameter (i.e., getParam<ReporterName>(param_name)).
   * @param mode The mode that the object will consume the Reporter value
   * @pararm time_index (optional) If zero is provided the current value is returned. Use a positive
   *                    index to return previous values (1 = older, 2 = older, etc.). The maximum
   *                    number of old values is dictated by the ReporterData object.
   */
  template <typename T>
  const T & getReporterValue(const std::string & param_name, const std::size_t time_index = 0);
  template <typename T>
  const T & getReporterValue(const std::string & param_name,
                             ReporterMode mode,
                             const std::size_t time_index = 0);
  // doco-normal-methods-end
  ///@}

  ///@{
  /**
   * Returns read-only reference to a Reporter value that is provided by name directly.
   * @tparam T The C++ type of the Reporter value being consumed
   * @param reporter_name A ReporterName object that for the desired Reporter value.
   * @param mode The mode that the object will consume the Reporter value
   * @pararm time_index (optional) If zero is provided the current value is returned. Use a positive
   *                    index to return previous values (1 = older, 2 = older, etc.). The maximum
   *                    number of old values is dictated by the ReporterData object.
   */
  template <typename T>
  const T & getReporterValueByName(const ReporterName & reporter_name,
                                   const std::size_t time_index = 0);
  template <typename T>
  const T & getReporterValueByName(const ReporterName & reporter_name,
                                   ReporterMode mode,
                                   const std::size_t time_index = 0);
  ///@}

  ///@{
  /**
   * Return True if the Reporter value exists.
   * @tparam T The C++ type of the Reporter value being consumed
   * @param reporter_name A ReporterName object that for the desired Reporter value.
   */
  bool hasReporterValue(const std::string & param_name) const;
  bool hasReporterValueByName(const ReporterName & reporter_name) const;
  template <typename T>
  bool hasReporterValue(const std::string & param_name) const;
  template <typename T>
  bool hasReporterValueByName(const ReporterName & reporter_name) const;

  ///@}

  /**
   * @returns The ReporterName associated with the parametre \p param_name.
   *
   * Performs error checking to mak sure that the parameter is valid.
   */
  const ReporterName & getReporterName(const std::string & param_name) const;

  /**
   * A method that can be overridden to update the UO dependencies.
   *
   * This is needed because the get methods for this interface cannot be virtual because of the
   * template parameter. See GeneralUserObject for how it is utilized.
   */
  virtual void addReporterDependencyHelper(const ReporterName & /*state_name*/) {}

private:
  /**
   * @returns True if all Reporters have been added (the task associated with adding them is
   * complete)
   */
  bool reportersAdded() const;

  /**
   * Helpers for "possibly" checking if a Reporter value exists. This is only
   * able to check for existance after all Reporters have been added (after
   * the task creating them has been called). If called before said task, this
   * will do nothing, hence the "possibly". This allows us to have errors reported
   * directly by the object requesting the Reporter instead of through a system with
   * less context.
   */
  template <typename T>
  void possiblyCheckHasReporter(const ReporterName & reporter_name,
                                const std::string & param_name = "") const;

  /// Parameters for the MooseObject inherting from this interface
  const InputParameters & _ri_params;

  /// Provides access to FEProblemBase::getReporterData
  FEProblemBase & _ri_fe_problem_base;

  /// The ReporterData
  const ReporterData & _ri_reporter_data;

  /// The MooseObject needing this interface
  const MooseObject & _ri_moose_object;
};

template <typename T>
const T &
ReporterInterface::getReporterValue(const std::string & param_name, const std::size_t time_index)
{
  return getReporterValue<T>(param_name, REPORTER_MODE_UNSET, time_index);
}

template <typename T>
const T &
ReporterInterface::getReporterValue(const std::string & param_name,
                                    ReporterMode mode,
                                    const std::size_t time_index)
{
  const auto & reporter_name = getReporterName(param_name);

  possiblyCheckHasReporter<T>(reporter_name, param_name);

  return getReporterValueByName<T>(reporter_name, mode, time_index);
}

template <typename T>
const T &
ReporterInterface::getReporterValueByName(const ReporterName & reporter_name,
                                          const std::size_t time_index)
{
  return getReporterValueByName<T>(reporter_name, REPORTER_MODE_UNSET, time_index);
}

template <typename T>
const T &
ReporterInterface::getReporterValueByName(const ReporterName & reporter_name,
                                          ReporterMode mode,
                                          const std::size_t time_index)
{
  possiblyCheckHasReporter<T>(reporter_name);

  addReporterDependencyHelper(reporter_name);

  return _ri_reporter_data.getReporterValue<T>(reporter_name, _ri_moose_object, mode, time_index);
}

template <typename T>
bool
ReporterInterface::hasReporterValue(const std::string & param_name) const
{
  if (!reportersAdded())
    _ri_moose_object.mooseError(
        "Cannot call hasReporterValue() until all Reporters have been constructed.");

  return hasReporterValueByName<T>(getReporterName(param_name));
}

template <typename T>
bool
ReporterInterface::hasReporterValueByName(const ReporterName & reporter_name) const
{
  if (!reportersAdded())
    _ri_moose_object.mooseError(
        "Cannot call hasReporterValueByName() until all Reporters have been constructed.");

  return _ri_reporter_data.hasReporterValue<T>(reporter_name);
}

template <typename T>
void
ReporterInterface::possiblyCheckHasReporter(const ReporterName & reporter_name,
                                            const std::string & param_name /* = "" */) const
{
  if (reportersAdded() && !hasReporterValueByName<T>(reporter_name))
  {
    std::stringstream oss;
    oss << "A Reporter value with the name \"" << reporter_name << "\" and type \""
        << MooseUtils::prettyCppType<T>() << "\" was not found.";

    if (_ri_params.isParamValid(param_name))
      _ri_moose_object.paramError(param_name, oss.str());
    else
      _ri_moose_object.mooseError(oss.str());
  }
}
