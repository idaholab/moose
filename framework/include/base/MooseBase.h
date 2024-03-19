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

class InputParameters;
class MooseApp;

#define usingMooseBaseMembers                                                                      \
  using MooseBase::getMooseApp;                                                                    \
  using MooseBase::type;                                                                           \
  using MooseBase::name;                                                                           \
  using MooseBase::typeAndName;                                                                    \
  using MooseBase::_type;                                                                          \
  using MooseBase::_app;                                                                           \
  using MooseBase::_name

/**
 * Base class for everything in MOOSE with a name and a type.
 * You will most likely want to inherit instead
 * - MooseObject for an object created within a system
 * - Action for a class performing a setup task, like creating objects
 */
class MooseBase
{
public:
  MooseBase(const std::string & type,
            const std::string & name,
            MooseApp & app,
            const InputParameters & params);

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
  std::string typeAndName() const;

  /**
   * @returns A prefix to be used in errors that contains the input
   * file location associated with this object (if any) and the
   * name and type of the object.
   */
  std::string errorPrefix(const std::string & error_type) const;

  /**
   * Calls moose error with the message \p msg.
   *
   * Will prefix the message with the subapp name if one exists.
   *
   * If \p with_prefix, then add the prefix from errorPrefix()
   * to the error.
   */
  [[noreturn]] void callMooseError(std::string msg, const bool with_prefix) const;

protected:
  /// The MOOSE application this is associated with
  MooseApp & _app;

  /// The type of this class
  const std::string _type;

  /// The name of this class
  const std::string _name;

private:
  /// The object's parameteres
  const InputParameters & _params;
};
