#pragma once

// MOOSE includes
#include "MultiAppTransfer.h"
#include "ReporterInterface.h"

// Forward declarations
class ControlsReceiver;

class OptimizationParameterTransfer : public MultiAppTransfer, public ReporterInterface
{
public:
  static InputParameters validParams();

  OptimizationParameterTransfer(const InputParameters & parameters);

  virtual void initialSetup() override;
  virtual void execute() override;

private:
  /**
   * Return the ControlsReceiver object and perform error checking.
   * @param app_index The global sup-app index
   */
  ControlsReceiver * getReceiver(unsigned int app_index);

  /// Value names from OptimizationReporter
  const std::vector<ReporterValueName> & _value_names;
  /// Parameter names in sub-application
  const std::vector<std::string> & _parameters;
  /// The name of the ControlsReceiver Control object on the sub-application
  const std::string & _receiver_name;

  /// Parameter values from OptimizationReporter
  std::vector<const std::vector<Real> *> _values;
};
