//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "FunctorInterface.h"

/**
 * \class ADFunctorInterface
 * \brief An interface for accessing \p Moose::Functors for systems that care about automatic
 * differentiation, e.g. AD kernels
 */
class ADFunctorInterface : public FunctorInterface
{
public:
  static InputParameters validParams();

  ADFunctorInterface(const MooseObject * moose_object);

private:
  virtual bool isADObject() const override { return true; }
};
