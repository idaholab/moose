//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "OutputInterface.h"
#include "FEProblemBase.h"
#include "NonADFunctorInterface.h"
#include "libmesh/parallel.h"

/**
 * Base class for all Postprocessors.  Defines a name and sets up the
 * virtual getValue() interface which must be overridden by derived
 * classes.
 */
class Postprocessor : public OutputInterface, public NonADFunctorInterface
{
public:
  static InputParameters validParams();

  Postprocessor(const MooseObject * moose_object);

  /**
   * This will get called to actually grab the final value the postprocessor has calculated.
   */
  virtual PostprocessorValue getValue() = 0;

  /**
   * Returns the name of the Postprocessor.
   */
  std::string PPName() const { return _pp_name; }

protected:
  const std::string _pp_name;
};
