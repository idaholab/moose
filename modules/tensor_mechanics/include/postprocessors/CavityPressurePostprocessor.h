/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef CAVITYPRESSUREPOSTPROCESSOR_H
#define CAVITYPRESSUREPOSTPROCESSOR_H

#include "GeneralPostprocessor.h"

class CavityPressureUserObject;

class CavityPressurePostprocessor : public GeneralPostprocessor
{
public:
  CavityPressurePostprocessor(const InputParameters & parameters);

  virtual ~CavityPressurePostprocessor() {}

  virtual void initialize() {}

  virtual void execute() {}

  virtual PostprocessorValue getValue();

protected:
  const CavityPressureUserObject & _cpuo;

  const std::string _quantity;
};

template <>
InputParameters validParams<CavityPressurePostprocessor>();

#endif
