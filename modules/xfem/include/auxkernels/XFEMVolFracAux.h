//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "AuxKernel.h"

class XFEM;

/**
 * Coupled auxiliary value
 */
class XFEMVolFracAux : public AuxKernel
{
public:
  /**
   * Factory constructor, takes parameters so that all derived classes can be built using the same
   * constructor.
   */
  static InputParameters validParams();

  XFEMVolFracAux(const InputParameters & parameters);

  virtual ~XFEMVolFracAux() {}

protected:
  virtual Real computeValue();

private:
  std::shared_ptr<XFEM> _xfem;
};
