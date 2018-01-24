/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef XFEMMARKERAUX_H
#define XFEMMARKERAUX_H

#include "AuxKernel.h"

class XFEM;

class XFEMMarkerAux : public AuxKernel
{
public:
  XFEMMarkerAux(const InputParameters & parameters);

  virtual ~XFEMMarkerAux() {}

protected:
  virtual Real computeValue();

private:
  MooseSharedPointer<XFEM> _xfem;
};

template <>
InputParameters validParams<XFEMMarkerAux>();

#endif // XFEMMARKERAUX_H
