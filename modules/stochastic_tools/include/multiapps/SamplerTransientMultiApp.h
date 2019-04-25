//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// MOOSE includes
#include "TransientMultiApp.h"
#include "SamplerInterface.h"

class SamplerTransientMultiApp;
class Sampler;

template <>
InputParameters validParams<SamplerTransientMultiApp>();

class SamplerTransientMultiApp : public TransientMultiApp, public SamplerInterface
{
public:
  SamplerTransientMultiApp(const InputParameters & parameters);

  /**
   * Return the Sampler object for this MultiApp.
   */
  Sampler & getSampler() const { return _sampler; }

protected:
  /// Sampler to utilize for creating MultiApps
  Sampler & _sampler;
};

