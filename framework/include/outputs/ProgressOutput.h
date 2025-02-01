//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Output.h"

class TransientBase;

/**
 * Output a simulation time progress bar on the console
 */
class ProgressOutput : public Output
{
public:
  static InputParameters validParams();

  ProgressOutput(const InputParameters & parameters);

protected:
  void output() override;

  const TransientBase * const _transient_executioner;

  /// display input file name in the progress bar title
  const bool _use_filename;

  /// total length of the progress bar
  const unsigned int _length;
};
