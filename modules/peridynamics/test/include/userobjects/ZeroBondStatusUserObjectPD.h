//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GeneralUserObjectBasePD.h"

/**
 * User object to set the bond status to zero for a given list of bond IDs.
 */
class ZeroBondStatusUserObjectPD : public GeneralUserObjectBasePD
{
public:
  static InputParameters validParams();

  ZeroBondStatusUserObjectPD(const InputParameters & parameters);

protected:
  virtual void initialize() override;
  virtual void execute() override;
  virtual void finalize() override;

  /// user specified bond IDs list
  std::vector<unsigned int> _list;
};
