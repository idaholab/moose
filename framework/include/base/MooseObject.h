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

#ifndef MOOSEOBJECT_H
#define MOOSEOBJECT_H

#include "InputParameters.h"
#include "ExecStore.h"

class MooseObject;

template<>
InputParameters validParams<MooseObject>();

/**
 * Every object that can be built by the factory should be derived from this class.
 */
class MooseObject
{
public:
  MooseObject(const std::string & name, InputParameters parameters);

  virtual ~MooseObject() { }

  const std::string & name() { return _name; }

  InputParameters & parameters() { return _pars; }

  template <typename T>
  const T & getParam(const std::string & name) { return _pars.get<T>(name); }

  template <typename T>
  const T & getParam(const std::string & name) const { return _pars.get<T>(name); }

  /// Gets called at the beginning of the simulation before this object is asked to do its job
  virtual void initialSetup() {}

  /// Gets called at the beginning of the timestep before this object is asked to do its job
  virtual void timestepSetup() {}

  /// Gets called just before the jacobian is computed and before this object is asked to do its job
  virtual void jacobianSetup() {}

  /// Gets called just before the residual is computed and before this object is asked to do its job
  virtual void residualSetup() {}

  /// Gets called when the subdomain changes (ie in a jacobian or residual loop) and before this object is asked to do its job
  virtual void subdomainSetup() {}

  virtual ExecFlagType execFlag() { return _exec_flags; }
  virtual void execFlag(ExecFlagType type) { _exec_flags = type; }

protected:
  std::string _name;
  InputParameters _pars;
  ExecFlagType _exec_flags;
};

#endif /* MOOSEOBJECT_H*/
