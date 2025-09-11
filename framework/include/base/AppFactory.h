//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include <vector>

#include "MooseApp.h"
#include "Capabilities.h"

#ifdef MOOSE_UNIT_TEST
#include "gtest/gtest.h"
class GTEST_TEST_CLASS_NAME_(AppFactoryTest, manageAppParams);
class GTEST_TEST_CLASS_NAME_(AppFactoryTest, appCopyConstructParams);
class GTEST_TEST_CLASS_NAME_(AppFactoryTest, createNotRegistered);
#endif

/**
 * Macros
 */
#define registerApp(name) AppFactory::instance().reg<name>(#name, __FILE__, __LINE__)

/**
 * Polymorphic data structure with parameter and object build access.
 */
struct AppFactoryBuildInfoBase
{
  virtual std::unique_ptr<MooseApp> build(const InputParameters & params) const = 0;
  virtual InputParameters buildParameters() const = 0;
  virtual ~AppFactoryBuildInfoBase() = default;

  /// Number of times this app has been created
  std::size_t app_creation_count = 0;
  /// C++ type of the app
  std::string type;
  /// File that registered this app
  std::string file;
  /// Line number in \p file that this app was registered
  int line;
};
template <typename T>
struct AppFactoryBuildInfo : public AppFactoryBuildInfoBase
{
  std::unique_ptr<MooseApp> build(const InputParameters & params) const override final
  {
    return std::make_unique<T>(params);
  }
  InputParameters buildParameters() const override final { return T::validParams(); }
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
   * @return Reference to the AppFactory instance
   */
  static AppFactory & instance();

  virtual ~AppFactory();

  static InputParameters validParams();

  /// The name for the "main" moose application
  static const std::string main_app_name;

  /**
   * Create an app with no input and command line arguments.
   *
   * Useful for unit testing.
   */
  static std::unique_ptr<MooseApp> create(const std::string & app_type,
                                          const std::vector<std::string> & cli_args = {});

  /**
   * Create a MooseApp from a Parser and CommandLine, both of which should have parsed.
   */
  static std::unique_ptr<MooseApp> create(std::unique_ptr<Parser> parser,
                                          std::unique_ptr<CommandLine> command_line);

  /**
   * Create a MooseApp given a set of parameters.
   *
   * The Parser must be set in the _parser param and the CommandLine must be set
   * in the _command_line param, both of which must have been parsed.
   *
   * @param app_type Type of the application being constructed
   * @param name Name for the object
   * @param parameters Parameters this object should have
   * @param throw_on_extract_error Whether or not to throw a ParseError instead of using mooseError
   * when [Application] block param extraction fails
   * @return The created object
   */
  ///@{
  std::unique_ptr<MooseApp> create(const std::string & app_type,
                                   const std::string & name,
                                   InputParameters parameters,
                                   MPI_Comm COMM_WORLD_IN,
                                   const bool throw_on_extract_error = false);
  std::shared_ptr<MooseApp> createShared(const std::string & app_type,
                                         const std::string & name,
                                         InputParameters parameters,
                                         MPI_Comm COMM_WORLD_IN);
  ///@}

  /**
   * Deprecated helper function for creating a MooseApp for Apps haven't adapted to the new Parser
   * and Builder changes. This function needed to be removed after the new Parser and Builder merged
   */
  static std::shared_ptr<MooseApp> createAppShared(const std::string & default_app_type,
                                                   int argc,
                                                   char ** argv,
                                                   MPI_Comm comm_word = MPI_COMM_WORLD);

  /**
   * Register a new object
   * @param name Name of the object to register
   */
  template <typename T>
  void reg(const std::string & name, const std::string & file, const int line);

  /**
   * Get valid parameters for the object
   * @param name Name of the object whose parameter we are requesting
   * @return Parameters of the object
   */
  InputParameters getValidParams(const std::string & name);

  /**
   * @return The parameters for the application named \p name
   *
   * This is needed because we poorly decided to not pass references
   * of the InputParameters in all derived MooseApp objects. This enables
   * the MooseApp to get the copy of the parameters that it was actually
   * built with using this factory.
   */
  const InputParameters & getAppParams(const InputParameters & params) const;

  /**
   * Class that is used as a parameter to clearAppParams() that allows only
   * MooseApp to call clearAppParams().
   */
  class ClearAppParamsKey
  {
    friend class MooseApp;
#ifdef MOOSE_UNIT_TEST
    FRIEND_TEST(::AppFactoryTest, manageAppParams);
#endif
    ClearAppParamsKey() {}
    ClearAppParamsKey(const ClearAppParamsKey &) {}
  };

  /**
   * Clears the stored parameters for the given application parameteres
   *
   * See getAppParams() for why this is needed.
   */
  void clearAppParams(const InputParameters & params, const ClearAppParamsKey);

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

protected:
  AppFactoryBuildInfoMap _name_to_build_info;

private:
  /**
   * Stores the given parameters within _input_parameters for app construction
   *
   * Also calls finalize() on the parameters
   */
  const InputParameters & storeAppParams(InputParameters & params);

  /**
   * Get the ID for the InputParameters associated with an application, used
   * in storing them in _input_parameters.
   *
   * This is needed until app constructors do not copy construct parameters.
   * See getAppParams() for more information.
   *
   * The parameters passed in here (from the app) could be copy-constructed
   * parameters, but will contain a "_app_params_id" parameter that allows
   * us to get the actual parameters (owned by this factory).
   */
  std::size_t getAppParamsID(const InputParameters & params) const;

  /// Private constructor for singleton pattern
  AppFactory() {}

#ifdef MOOSE_UNIT_TEST
  FRIEND_TEST(::AppFactoryTest, manageAppParams);
  FRIEND_TEST(::AppFactoryTest, appCopyConstructParams);
  FRIEND_TEST(::AppFactoryTest, createNotRegistered);
#endif

  /// Storage of input parameters used in applications (ID (from getAppParamsID()) -> params)
  std::map<std::size_t, std::unique_ptr<InputParameters>> _input_parameters;
};

template <typename T>
void
AppFactory::reg(const std::string & name, const std::string & file, const int line)
{
  if (isRegistered(name))
    return;

  auto it_inserted_pair =
      _name_to_build_info.emplace(name, std::make_unique<AppFactoryBuildInfo<T>>());
  auto & info = *it_inserted_pair.first->second;
  info.type = name;
  info.file = file;
  info.line = line;

  Moose::Capabilities::getCapabilityRegistry().add(
      name, true, "MOOSE application " + name + " is available.");
}
