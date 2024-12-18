//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Action.h"

/**
 * Input parameters common to all block-restricted NEML2Actions
 *
 * For example, in the input file, we could have
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * [NEML2]
 *   a = 1
 *   b = 2
 *   c = 3
 *   [block1]
 *     d = 4
 *     e = 5
 *     block = 1
 *   []
 *   [block2]
 *     d = 6
 *     e = 7
 *     block = 2
 *   []
 *   [block3]
 *     d = 8
 *     e = 9
 *     block = 3
 *   []
 * []
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 *
 * This object defines input parameters a, b, and c, which will be applied to each of the
 * block-restricted NEML2Action: block1, block2, and block3.
 */
class NEML2ActionCommon : public Action
{
public:
  /// Parameters that can be specified EITHER under the common area OR under sub-blocks
  static InputParameters commonParams();

  /// Parameters that can ONLY be specified under the common area
  static InputParameters validParams();

  NEML2ActionCommon(const InputParameters &);

  virtual void act() override;

  const FileName & fname() const { return _fname; }

protected:
  /// Name of the NEML2 input file
  const FileName _fname;

  /// List of cli-args
  const std::vector<std::string> _cli_args;
};
