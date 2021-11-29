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
  virtual void initialSetup() { _counts["initial"]++; }
  virtual void timestepSetup() { _counts["timestep"]++; }
  virtual void residualSetup() { _counts["linear"]++; }
  virtual void jacobianSetup() { _counts["nonlinear"]++; }
  virtual void initialize();
  virtual void finalize();
  virtual void execute() { _execute++; }
  ///@}

  ///@{
  /**
   *  Helper functions to account for final on subdomainSetup and threadJoin
   */
  void subdomainSetupHelper() { _counts["subdomain"]++; }
  void threadJoinHelper(const UserObject & uo);
  ///@}

  /**
   * Return the count base on the count type supplied in the input file.
   */
  PostprocessorValue getValue();

private:
  /// The type of count to report
  MooseEnum _count_type;

  /// Local count of execute (allows execute count to work with parallel and threading)
  unsigned int _execute;

  /// Storage for the various counts
  std::map<std::string, unsigned int> & _counts;
};

template <class T>
InputParameters
SetupInterfaceCount<T>::validParams()
{
  InputParameters parameters = T::validParams();
  MooseEnum count_type(
      "initial timestep subdomain linear nonlinear initialize finalize execute threadjoin");
  parameters.addRequiredParam<MooseEnum>(
      "count_type", count_type, "Specify the count type to return.");
  return parameters;
}

template <class T>
SetupInterfaceCount<T>::SetupInterfaceCount(const InputParameters & parameters)
  : T(parameters),
    _count_type(T::template getParam<MooseEnum>("count_type")),
    _execute(0),
    _counts(T::template declareRestartableData<std::map<std::string, unsigned int>>("counts"))
{
  // Initialize the count storage map
  const std::vector<std::string> & names = _count_type.getNames();
  for (std::vector<std::string>::const_iterator it = names.begin(); it != names.end(); ++it)
    _counts[*it] = 0;
}

template <class T>
PostprocessorValue
SetupInterfaceCount<T>::getValue()
{
  unsigned int count = _counts[_count_type];
  return count;
}

template <class T>
void
SetupInterfaceCount<T>::initialize()
{
  _counts["initialize"]++;
  _execute = 0;
}

template <class T>
void
SetupInterfaceCount<T>::finalize()
{
  T::gatherSum(_execute);
  _counts["execute"] += _execute;
  _counts["finalize"]++;
}

template <class T>
void
SetupInterfaceCount<T>::threadJoinHelper(const UserObject & uo)
{
  // Accumulate 'execute' count from other threads
  const SetupInterfaceCount<T> & sic = static_cast<const SetupInterfaceCount<T> &>(uo);
  _execute += sic._execute;
  _counts["threadjoin"]++;
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
