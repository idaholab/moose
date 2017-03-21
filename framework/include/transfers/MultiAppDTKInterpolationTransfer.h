/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

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
