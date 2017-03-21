/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef XFEMVOLFRACAUX_H
#define XFEMVOLFRACAUX_H

#include "AuxKernel.h"

class XFEM;

/**
 * Coupled auxiliary value
 */
class XFEMVolFracAux : public AuxKernel
{
public:
  /**
   * Factory constructor, takes parameters so that all derived classes can be built using the same
   * constructor.
   */
  XFEMVolFracAux(const InputParameters & parameters);

  virtual ~XFEMVolFracAux() {}

protected:
  virtual Real computeValue();

private:
  MooseSharedPointer<XFEM> _xfem;
};

template <>
InputParameters validParams<XFEMVolFracAux>();

#endif // XFEMVOLFRACAUX_H
