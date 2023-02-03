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
 * \class NonADFunctorInterface
 * \brief An interface for accessing \p Moose::Functors for systems that do not care about automatic
 * differentiation, e.g. auxiliary kernels
 */
class NonADFunctorInterface : public FunctorInterface
{
public:
  static InputParameters validParams();

  NonADFunctorInterface(const MooseObject * moose_object);

private:
  virtual bool isADObject() const override { return false; }
};
