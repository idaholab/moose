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

class MultiAppVppToVppTransfer : public MultiAppTransfer
{
public:
  static InputParameters validParams();

  MultiAppVppToVppTransfer(const InputParameters & parameters);

  virtual void execute() override;

  virtual void initialSetup() override;

private:
  void initialSetupFromMultiapp();
  void initialSetupToMultiapp();

  void executeFromMultiapp();
  void executeToMultiapp();

  void copyVectorPostprocessors(FEProblemBase & fe_base, const VectorPostprocessorName & vpp_name);

  const VectorPostprocessorName & _sub_vpp_name;
  const VectorPostprocessorName & _master_vpp_name;
  std::vector<VectorPostprocessorValue *> _receiver_vpps;
};
