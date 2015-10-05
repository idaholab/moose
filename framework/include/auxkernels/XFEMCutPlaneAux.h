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

#ifndef XFEMCUTPLANEAUX_H
#define XFEMCUTPLANEAUX_H

#include "AuxKernel.h"

class XFEM;

/**
 * Coupled auxiliary value
 */
class XFEMCutPlaneAux : public AuxKernel
{
public:

  /**
   * Factory constructor, takes parameters so that all derived classes can be built using the same
   * constructor.
   */
  XFEMCutPlaneAux(const InputParameters & parameters);

  virtual ~XFEMCutPlaneAux() {}

protected:
  virtual Real computeValue();

private:
  XFEM_CUTPLANE_QUANTITY _quantity;
  XFEM *_xfem;
  unsigned int _plane_id;
};

template<>
InputParameters validParams<XFEMCutPlaneAux>();

#endif //XFEMCUTPLANEAUX_H
