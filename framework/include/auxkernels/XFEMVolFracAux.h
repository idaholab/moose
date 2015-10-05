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
  XFEM *_xfem;
};

template<>
InputParameters validParams<XFEMVolFracAux>();

#endif //XFEMVOLFRACAUX_H
