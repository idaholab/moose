//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MultiAppTransfer.h"

template <typename ReporterType>
class MultiAppReporterTransfer;

typedef MultiAppReporterTransfer<int> MultiAppIntegerReporterTransfer;
typedef MultiAppReporterTransfer<Real> MultiAppRealReporterTransfer;
typedef MultiAppReporterTransfer<std::vector<Real>> MultiAppVectorReporterTransfer;
typedef MultiAppReporterTransfer<std::string> MultiAppStringReporterTransfer;

template <typename ReporterType>
class MultiAppReporterTransfer : public MultiAppTransfer
{
public:
  static InputParameters validParams();

  MultiAppReporterTransfer(const InputParameters & parameters);

  virtual void execute() override;

protected:
  virtual void executeToMultiapp();
  virtual void executeFromMultiapp();

  const std::vector<ReporterName> & _from_reporter_names;
  const std::vector<ReporterName> & _to_reporter_names;
  const unsigned int & _subapp_index;
};
