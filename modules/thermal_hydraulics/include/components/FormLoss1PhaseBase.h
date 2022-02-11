//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Component.h"

/**
 * Base class for prescribing a form loss over a 1-phase flow channel
 */
class FormLoss1PhaseBase : public Component
{
public:
  FormLoss1PhaseBase(const InputParameters & params);

  virtual void init() override;
  virtual void check() const override;
  virtual void addMooseObjects() override;

protected:
  /// Subdomains corresponding to the connected flow channel
  std::vector<SubdomainName> _flow_channel_subdomains;
  /// Name of the flow channel component
  const std::string & _flow_channel_name;

public:
  static InputParameters validParams();
};
