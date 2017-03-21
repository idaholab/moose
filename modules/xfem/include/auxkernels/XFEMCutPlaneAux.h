/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

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
