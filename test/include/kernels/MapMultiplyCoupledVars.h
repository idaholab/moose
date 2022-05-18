//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADKernel.h"

class MapMultiplyCoupledVars : public ADKernel
{
public:
  static InputParameters validParams();

  MapMultiplyCoupledVars(const InputParameters & parameters);

protected:
  virtual ADReal computeQpResidual() override;

private:
  const ADVariableValue & _v;
  const ADVariableValue & _w;
  const std::map<std::string, Real> & _coupled_map;
  const std::map<std::string, std::string> & _dummy_string_to_string_map;
  const std::map<unsigned long long, unsigned int> & _dummy_ullong_to_uint_map;
  const std::map<unsigned int, unsigned int> & _dummy_uint_to_uint_map;
  const std::map<unsigned long, unsigned int> & _dummy_ulong_to_uint_map;
  Real _v_multiplier;
  Real _w_multiplier;
};
