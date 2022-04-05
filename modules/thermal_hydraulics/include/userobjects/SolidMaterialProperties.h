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
#include "Function.h"

/**
 *
 */
class SolidMaterialProperties : public GeneralUserObject
{
public:
  SolidMaterialProperties(const InputParameters & parameters);

  virtual void initialize();
  virtual void finalize();
  virtual void execute();

  ADReal k(const ADReal & temp) const;
  ADReal cp(const ADReal & temp) const;
  ADReal rho(const ADReal & temp) const;

  const Function & getKFunction() const { return _k; }
  const Function & getCpFunction() const { return _cp; }
  const Function & getRhoFunction() const { return _rho; }

protected:
  const Function & _k;
  const Function & _cp;
  const Function & _rho;

public:
  static InputParameters validParams();
};
