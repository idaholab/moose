//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef GHOSTINGAUX_H
#define GHOSTINGAUX_H

#include "AuxKernel.h"

#include "libmesh/ghosting_functor.h"

// Forward Declarations
class GhostingAux;

template <>
InputParameters validParams<GhostingAux>();

/**
 *
 */
class GhostingAux : public AuxKernel
{
public:
  GhostingAux(const InputParameters & parameters);

  virtual void initialize() override;

protected:
  virtual Real computeValue() override;

  /// The PID to show the ghosting for
  processor_id_type _pid;

  /// The type of ghosting functor to get
  int _functor_type;

  /// Ghosted elems
  libMesh::GhostingFunctor::map_type _ghosted_elems;
};

#endif // GHOSTINGAUX_H
