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

#ifndef SETUPINTERFACECOUT_H
#define SETUPINTERFACECOUT_H

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

template <>
InputParameters validParams<GeneralSetupInterfaceCount>();
template <>
InputParameters validParams<ElementSetupInterfaceCount>();
template <>
InputParameters validParams<SideSetupInterfaceCount>();
template <>
InputParameters validParams<InternalSideSetupInterfaceCount>();
template <>
InputParameters validParams<NodalSetupInterfaceCount>();

/**
 * A class for testing the number of calls to the various SetupInterface methods.
 */
template <class T>
class SetupInterfaceCount : public T
{
public:
  SetupInterfaceCount(const InputParameters & parameters);

  ///@{
  /**
   * Each setup methods simply increments a counter.
   */
  virtual void initialSetup() { _counts["initial"]++; }
  virtual void timestepSetup() { _counts["timestep"]++; }
  virtual void subdomainSetup() { _counts["subdomain"]++; }
  virtual void residualSetup() { _counts["linear"]++; }
  virtual void jacobianSetup() { _counts["nonlinear"]++; }
  virtual void initialize();
  virtual void finalize();
  virtual void execute() { _execute++; }
  virtual void threadJoin(const UserObject & uo);

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
SetupInterfaceCount<T>::threadJoin(const UserObject & uo)
{
  // Accumulate 'execute' count from other threads
  const SetupInterfaceCount<T> & sic = static_cast<const SetupInterfaceCount<T> &>(uo);
  _execute += sic._execute;
  _counts["threadjoin"]++;
}

template <class T>
void
SetupInterfaceCount<T>::finalize()
{
  T::gatherSum(_execute);
  _counts["execute"] += _execute;
  _counts["finalize"]++;
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
};

class SideSetupInterfaceCount : public SetupInterfaceCount<SidePostprocessor>
{
public:
  SideSetupInterfaceCount(const InputParameters & parameters);
};

class InternalSideSetupInterfaceCount : public SetupInterfaceCount<InternalSidePostprocessor>
{
public:
  InternalSideSetupInterfaceCount(const InputParameters & parameters);
};

class NodalSetupInterfaceCount : public SetupInterfaceCount<NodalPostprocessor>
{
public:
  NodalSetupInterfaceCount(const InputParameters & parameters);
};

#endif // SETUPINTERFACECOUT_H
