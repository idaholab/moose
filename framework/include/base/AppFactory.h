//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "InputParameters.h"

class MooseApp;

/**
 * Macros
 */
#define registerApp(name) AppFactory::instance().reg<name>(#name)

/**
 * alias to wrap shared pointer type
 */
using MooseAppPtr = std::shared_ptr<MooseApp>;

/**
 * Polymorphic data structure with parameter and object build access.
 */
struct AppFactoryBuildInfoBase
{
  virtual MooseAppPtr build(const InputParameters & params) = 0;
  virtual InputParameters buildParameters() = 0;
  virtual ~AppFactoryBuildInfoBase() = default;

  std::size_t _app_creation_count = 0;
};
template <typename T>
struct AppFactoryBuildInfo : public AppFactoryBuildInfoBase
{
  virtual MooseAppPtr build(const InputParameters & params) override
  {
    return std::make_shared<T>(params);
  }
  virtual InputParameters buildParameters() override { return T::validParams(); }
};

using AppFactoryBuildInfoMap = std::map<std::string, std::unique_ptr<AppFactoryBuildInfoBase>>;

/**
 * Generic AppFactory class for building Application objects
 */
class AppFactory
{
public:
  /**
   * Get the instance of the AppFactory
   * @return Pointer to the AppFactory instance
   */
  static AppFactory & instance();

  /**
   * Register a new object
   * @param name Name of the object to register
   */
  template <typename T>
  void reg(const std::string & name);

  /**
   * Get valid parameters for the object
   * @param name Name of the object whose parameter we are requesting
   * @return Parameters of the object
   */
  InputParameters getValidParams(const std::string & name);

  /**
   * Creates an application with the parameters in \p parameters.
   *
   * @return The application
   */
  MooseAppPtr createShared(InputParameters parameters);

  /**
   * Returns a reference to the map from names to AppFactoryBuildInfo pointers
   */
  const auto & registeredObjects() const { return _name_to_build_info; }

  /**
   * Returns a Boolean indicating whether an application type has been registered
   */
  bool isRegistered(const std::string & app_name) const
  {
    return _name_to_build_info.count(app_name);
  }

  /**
   * @returns the amount of times the AppFactory created the named App-type
   */
  std::size_t createdAppCount(const std::string & app_type) const;

  /**
   * Returns the map of object name to a function pointer for building said object's
   * input parameters.
   */
  const AppFactoryBuildInfoMap & registeredObjectBuildInfos() const { return _name_to_build_info; }

  ///@{ Don't allow creation through copy/move construction or assignment
  AppFactory(AppFactory const &) = delete;
  Registry & operator=(AppFactory const &) = delete;

  AppFactory(AppFactory &&) = delete;
  Registry & operator=(AppFactory &&) = delete;
  ///@}

  /**
   * @return Whether or not this factory is currently constructing an object
   *
   * Used in the MooseApp constructor to enforce that construction must
   * only be done by the AppFactory
   */
  bool currentlyConstructing() const { return _currently_constructing; }

protected:
  AppFactoryBuildInfoMap _name_to_build_info;

  /// Whether or not we're currently constructing an app; see currentlyConstructing()
  bool _currently_constructing = false;

private:
  // Private constructor for singleton pattern
  AppFactory() {}
};

template <typename T>
void
AppFactory::reg(const std::string & name)
{
  if (isRegistered(name))
    return;

  _name_to_build_info[name] = std::make_unique<AppFactoryBuildInfo<T>>();
}
