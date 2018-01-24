//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef XFEMCUTPLANEAUX_H
#define XFEMCUTPLANEAUX_H

#include "AuxKernel.h"
#include "XFEM.h"

/**
 * Coupled auxiliary value
 */
class XFEMCutPlaneAux : public AuxKernel
{
public:
  XFEMCutPlaneAux(const InputParameters & parameters);

  virtual ~XFEMCutPlaneAux() {}

protected:
  virtual Real computeValue();

private:
  Xfem::XFEM_CUTPLANE_QUANTITY _quantity;
  MooseSharedPointer<XFEM> _xfem;
  unsigned int _plane_id;
};

template <>
InputParameters validParams<XFEMCutPlaneAux>();

#endif // XFEMCUTPLANEAUX_H
