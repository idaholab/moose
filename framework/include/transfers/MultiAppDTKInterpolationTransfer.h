//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef MULTIAPPDTKINTERPOLATIONTRANSFER_H
#define MULTIAPPDTKINTERPOLATIONTRANSFER_H

#include "libmesh/libmesh_config.h"

#ifdef LIBMESH_TRILINOS_HAVE_DTK

#include "MultiAppTransfer.h"
#include "DTKInterpolationHelper.h"

// Forward declarations
class MultiAppDTKInterpolationTransfer;

template <>
InputParameters validParams<MultiAppDTKInterpolationTransfer>();

/**
 * Transfers from spatially varying Interpolations in a MultiApp to the "master" system.
 */
class MultiAppDTKInterpolationTransfer : public MultiAppTransfer
{
public:
  MultiAppDTKInterpolationTransfer(const InputParameters & parameters);

  virtual void execute() override;

protected:
  VariableName _from_var_name;
  AuxVariableName _to_var_name;

  DTKInterpolationHelper _helper;
  Point _master_position;
};

#endif // LIBMESH_TRILINOS_HAVE_DTK
#endif // MULTIAPPDTKINTERPOLATIONTRANSFER_H
