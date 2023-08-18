//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// MOOSE includes
#include "GeneralPostprocessor.h"
#include "ElementPostprocessor.h"
#include "SidePostprocessor.h"
#include "InternalSidePostprocessor.h"
#include "NodalPostprocessor.h"

// Forward declarations
class GeneralSetupInterfaceCount;
class ElementSetupInterfaceCount;
class SideSetupInterfaceCount;
class InternalSideSetupInterfaceCount;
class NodalSetupInterfaceCount;
/**
 * A class for testing the number of calls to the various SetupInterface methods.
 */
template <class T>
class SetupInterfaceCount : public T
{
public:
  SetupInterfaceCount(const InputParameters & parameters);

  static InputParameters validParams();

  ///@{
  /**
   * Each setup methods simply increments a counter.
   */
  virtual void initialSetup() override;
  virtual void timestepSetup() override { _counts.at("TIMESTEP")++; }
  virtual void residualSetup() override { _counts.at("LINEAR")++; }
  virtual void jacobianSetup() override { _counts.at("NONLINEAR")++; }
  virtual void initialize() override;
  virtual void finalize() override;
  virtual void execute() override { _execute++; }
  ///@}

  ///@{
  /**
   *  Helper functions to account for final on subdomainSetup and threadJoin
   */
  void subdomainSetupHelper() { _counts.at("SUBDOMAIN")++; }
  void threadJoinHelper(const UserObject & uo);
  ///@}

  /**
   * Return the count base on the count type supplied in the input file.
   */
  using Postprocessor::getValue;
  virtual PostprocessorValue getValue() override;

private:
  /// The type of count to report
  MooseEnum _count_type;

  /// Local count of execute (allows execute count to work with parallel and threading)
  unsigned int _execute;

  /// Whether or not we called initialSetup once (not accounting for restart/recover)
  /// See initialSetup() as to why we need this
  bool _called_initial_setup;

  /// Storage for the various counts
  std::map<std::string, unsigned int> & _counts;
};

template <class T>
InputParameters
SetupInterfaceCount<T>::validParams()
{
  InputParameters parameters = T::validParams();
  MooseEnum count_type(
      "INITIAL TIMESTEP SUBDOMAIN LINEAR NONLINEAR INITIALIZE FINALIZE EXECUTE THREADJOIN");
  parameters.addRequiredParam<MooseEnum>(
      "count_type", count_type, "Specify the count type to return.");
  return parameters;
}

template <class T>
SetupInterfaceCount<T>::SetupInterfaceCount(const InputParameters & parameters)
  : T(parameters),
    _count_type(T::template getParam<MooseEnum>("count_type")),
    _execute(0),
    _called_initial_setup(false),
    _counts(T::template declareRestartableData<std::map<std::string, unsigned int>>("counts"))
{
  // Initialize the count storage map
  for (const auto & name : _count_type.getNames())
    _counts[name] = 0;
}

template <class T>
PostprocessorValue
SetupInterfaceCount<T>::getValue()
{
  return _counts.at(_count_type);
}

template <class T>
void
SetupInterfaceCount<T>::initialSetup()
{
  // In the case of restart/recover, we will _actually_ be doing more than one initial
  // setups... but, we want to support all of these tests with --recover and still have
  // them work. So, we will cheat and zero this whenever we're doing restart/recover
  // the first time
  if (!_called_initial_setup && (this->_app.isRestarting() || this->_app.isRecovering()))
    _counts.at("INITIAL") = 0;
  _called_initial_setup = true;
  _counts.at("INITIAL")++;
}

template <class T>
void
SetupInterfaceCount<T>::initialize()
{
  _counts.at("INITIALIZE")++;
  _execute = 0;
}

template <class T>
void
SetupInterfaceCount<T>::finalize()
{
  T::gatherSum(_execute);
  _counts.at("EXECUTE") += _execute;
  _counts.at("FINALIZE")++;
}

template <class T>
void
SetupInterfaceCount<T>::threadJoinHelper(const UserObject & uo)
{
  // Accumulate 'execute' count from other threads
  const SetupInterfaceCount<T> & sic = static_cast<const SetupInterfaceCount<T> &>(uo);
  _execute += sic._execute;
  _counts.at("THREADJOIN")++;
}

// Define objects for each of the UserObject base classes
class GeneralSetupInterfaceCount : public SetupInterfaceCount<GeneralPostprocessor>
{
public:
  GeneralSetupInterfaceCount(const InputParameters & parameters);
};

class ElementSetupInterfaceCount : public SetupInterfaceCount<ElementPostprocessor>
{
public:
  ElementSetupInterfaceCount(const InputParameters & parameters);

protected:
  virtual void threadJoin(const UserObject & uo) { threadJoinHelper(uo); }
  virtual void subdomainSetup() { subdomainSetupHelper(); }
};

class SideSetupInterfaceCount : public SetupInterfaceCount<SidePostprocessor>
{
public:
  SideSetupInterfaceCount(const InputParameters & parameters);

protected:
  virtual void threadJoin(const UserObject & uo) { threadJoinHelper(uo); }
  virtual void subdomainSetup() { subdomainSetupHelper(); }
};

class InternalSideSetupInterfaceCount : public SetupInterfaceCount<InternalSidePostprocessor>
{
public:
  InternalSideSetupInterfaceCount(const InputParameters & parameters);

protected:
  virtual void threadJoin(const UserObject & uo) { threadJoinHelper(uo); }
  virtual void subdomainSetup() { subdomainSetupHelper(); }
};

class NodalSetupInterfaceCount : public SetupInterfaceCount<NodalPostprocessor>
{
public:
  NodalSetupInterfaceCount(const InputParameters & parameters);

protected:
  virtual void threadJoin(const UserObject & uo) { threadJoinHelper(uo); }
  virtual void subdomainSetup() { subdomainSetupHelper(); }
};
