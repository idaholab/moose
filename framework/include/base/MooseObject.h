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
#include "ConsoleStreamInterface.h"

// libMesh includes
#include "libmesh/parallel_object.h"

class MooseApp;
class MooseObject;

template<>
InputParameters validParams<MooseObject>();

/**
 * Every object that can be built by the factory should be derived from this class.
 */
class MooseObject :
  public ConsoleStreamInterface,
  public libMesh::ParallelObject
{
public:
  MooseObject(const std::string & name, InputParameters parameters);

  virtual ~MooseObject() { }

  /**
   * Get the name of the object
   * @return The name of the object
   */
  const std::string & name() const { return _name; }

  /**
   * Get the parameters of the object
   * @return The parameters of the object
   */
  InputParameters & parameters() { return _pars; }

  ///@{
  /**
   * Retrieve a parameter for the object
   * @param name The name of the parameter
   * @return The value of the parameter
   */
  template <typename T>
  const T & getParam(const std::string & name);

  template <typename T>
  const T & getParam(const std::string & name) const;
  ///@}

  /**
   * Test if the supplied parameter is valid
   * @param name The name of the parameter to test
   */
  inline bool isParamValid(const std::string &name) const { return _pars.isParamValid(name); }

  /**
   * Get the MooseApp this object is associated with.
   */
  MooseApp & getMooseApp() { return _app; }

protected:

  /// The name of this object
  std::string _name;

  /// Parameters of this object
  InputParameters _pars;

  /// The MooseApp this object is associated with
  MooseApp & _app;
};

template <typename T>
const T &
MooseObject::getParam(const std::string & name)
{
  return InputParameters::getParamHelper(name, _pars, static_cast<T *>(0));
}

template <typename T>
const T &
MooseObject::getParam(const std::string & name) const
{
  return InputParameters::getParamHelper(name, _pars, static_cast<T *>(0));
}

#endif /* MOOSEOBJECT_H*/
