//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GeneralUserObject.h"

class TravelingSalesmanUO : public GeneralUserObject
{
public:
  static InputParameters validParams();

  TravelingSalesmanUO(const InputParameters & parameters);

  virtual void initialize() override {}
  virtual void execute() override;
  virtual void finalize() override {}

  static void obj(Real & objective,
                  const std::vector<Real> & rparams,
                  const std::vector<int> & iparams,
                  void * ctx);
};
