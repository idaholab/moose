//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include <string>

class MooseApp;

/**
 * Base class for everything in MOOSE with a name and a type.
 * You will most likely want to inherit instead
 * - MooseObject for an object created within a system
 * - Action for a class performing a setup task, like creating objects
 */
class MooseBase
{
public:
  MooseBase(const std::string & type, const std::string & name, MooseApp & app)
    : _app(app), _type(type), _name(name)
  {
  }

  virtual ~MooseBase() = default;

  /**
   * Get the MooseApp this class is associated with.
   */
  MooseApp & getMooseApp() const { return _app; }

  /**
   * Get the type of this class.
   * @return the name of the type of this class
   */
  const std::string & type() const { return _type; }

  /**
   * Get the name of the class
   * @return The name of the class
   */
  virtual const std::string & name() const { return _name; }

  /**
   * Get the class's combined type and name; useful in error handling.
   * @return The type and name of this class in the form '<type()> "<name()>"'.
   */
  std::string typeAndName() const
  {
    return type() + std::string(" \"") + name() + std::string("\"");
  }

protected:
  /// The MOOSE application this is associated with
  MooseApp & _app;

  /// The type of this class
  const std::string & _type;

  /// The name of this class, reference to value stored in InputParameters
  const std::string & _name;
};
