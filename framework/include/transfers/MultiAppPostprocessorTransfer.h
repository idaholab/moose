//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef MULTIAPPPOSTPROCESSORTRANSFER_H
#define MULTIAPPPOSTPROCESSORTRANSFER_H

#include "MultiAppTransfer.h"

// Forward declarations
class MultiAppPostprocessorTransfer;

template <>
InputParameters validParams<MultiAppPostprocessorTransfer>();

/**
 * Copies the value of a Postprocessor from the Master to a MultiApp.
 */
class MultiAppPostprocessorTransfer : public MultiAppTransfer
{
public:
  MultiAppPostprocessorTransfer(const InputParameters & parameters);

  virtual void execute() override;

  enum
  {
    AVERAGE,
    SUM,
    MAXIMUM,
    MINIMUM
  };

protected:
  PostprocessorName _from_pp_name;
  PostprocessorName _to_pp_name;
  MooseEnum _reduction_type;
};

#endif /* MULTIAPPPOSTPROCESSORTRANSFER_H */
