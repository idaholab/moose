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

#ifndef XFEMMARKERAUX_H
#define XFEMMARKERAUX_H

#include "AuxKernel.h"

class XFEM;

/**
 * Coupled auxiliary value
 */
class XFEMMarkerAux : public AuxKernel
{
public:

  /**
   * Factory constructor, takes parameters so that all derived classes can be built using the same
   * constructor.
   */
  XFEMMarkerAux(const InputParameters & parameters);

  virtual ~XFEMMarkerAux() {}

protected:
  virtual Real computeValue();

private:
  XFEM *_xfem;
};

template<>
InputParameters validParams<XFEMMarkerAux>();

#endif //XFEMMARKERAUX_H
