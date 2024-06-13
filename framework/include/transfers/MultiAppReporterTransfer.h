//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once
#include "ReporterTransferInterface.h"
#include "MultiAppTransfer.h"

/**
 * Transfer for migrating reporter values between the main and sub-application(s).
 */
class MultiAppReporterTransfer : public MultiAppTransfer, public ReporterTransferInterface
{
public:
  static InputParameters validParams();

  MultiAppReporterTransfer(const InputParameters & parameters);
  virtual void initialSetup() override;
  virtual void execute() override;

protected:
  virtual void executeToMultiapp();
  virtual void executeFromMultiapp();

  /// Vector of reporters to transfer data from
  const std::vector<ReporterName> & _from_reporter_names;

  /// Vector of reporters to transfer data to
  const std::vector<ReporterName> & _to_reporter_names;

  /// If set, indicates a particular subapp to transfer the reporter to/from
  const unsigned int & _subapp_index;

private:
  virtual void checkSiblingsTransferSupported() const override;
  /// determines transfer type
  bool _distribute_reporter_vector;

  /**
   * Sets transfer modes for reporters when distributing.
   * @param main_app The main app's shared pointer.
   * @param main_app_rep_names A vector containing the names of reporters in the main app.
   * @param sub_app_rep_names A vector containing the names of reporters in the sub apps.
   */
  void setVectorReporterTransferModes(const std::shared_ptr<MultiApp> & main_app,
                                      const std::vector<ReporterName> & main_app_rep_names,
                                      const std::vector<ReporterName> & sub_app_rep_names);
};
