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

  const std::vector<ReporterName> & _from_reporter_names;
  const std::vector<ReporterName> & _to_reporter_names;
  const unsigned int & _subapp_index;
};
