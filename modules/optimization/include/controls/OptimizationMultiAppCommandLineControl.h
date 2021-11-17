#pragma once

// MOOSE includes
#include "Control.h"
#include "ReporterInterface.h"

// Forward declarations
class MultiApp;

/**
 * A Control object for receiving data from a master application Sampler object.
 */
class OptimizationMultiAppCommandLineControl : public Control, public ReporterInterface
{
public:
  static InputParameters validParams();

  OptimizationMultiAppCommandLineControl(const InputParameters & parameters);

  /**
   * Do not allow the use of initialSetup, because this class is designed to operate
   * on PRE_MULTIAPP_SETUP, which occurs before this callback. This will prevent a child class
   * adding something to this function without it doing anything.
   */
  virtual void initialSetup() override final;

  virtual void execute() override;

protected:
  /// The MultiApp this Transfer is transferring data to or from
  std::shared_ptr<MultiApp> _multi_app;
  /// Value names from OptimizationReporter
  const std::vector<ReporterValueName> & _value_names;
  /// Parameter names in sub-application
  const std::vector<std::string> & _parameters;
};
