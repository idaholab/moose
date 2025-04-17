//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MoosePreconditioner.h"

// Forward declarations
class NonlinearSystemBase;
class InputParameters;

#include <vector>
#include <string>

/**
 * Implements a preconditioner designed to map onto PETSc's PCFieldSplit.
 */
template <typename Base>
class FieldSplitPreconditionerTempl : public Base
{
public:
  /**
   *  Constructor. Initializes SplitBasedPreconditioner data structures
   */
  static InputParameters validParams();

  FieldSplitPreconditionerTempl(const InputParameters & parameters);

  /**
   * top split
   */
  std::vector<std::string> _top_split;

protected:
  /// The nonlinear system this FSP is associated with (convenience reference)
  NonlinearSystemBase & _nl;
};

class FieldSplitPreconditioner : public FieldSplitPreconditionerTempl<MoosePreconditioner>
{
public:
  static InputParameters validParams();

  FieldSplitPreconditioner(const InputParameters & parameters);
};
