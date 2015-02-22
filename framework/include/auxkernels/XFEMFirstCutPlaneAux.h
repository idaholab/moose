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

#ifndef XFEMFIRSTCUTPLANEAUX_H
#define XFEMFIRSTCUTPLANEAUX_H

#include "AuxKernel.h"

class XFEM;

/**
 * Coupled auxiliary value
 */
class XFEMFirstCutPlaneAux : public AuxKernel
{
public:

  /**
   * Factory constructor, takes parameters so that all derived classes can be built using the same
   * constructor.
   */
  XFEMFirstCutPlaneAux(const std::string & name, InputParameters parameters);

  virtual ~XFEMFirstCutPlaneAux() {}

protected:
  virtual Real computeValue();

private:
  XFEM_CUTPLANE_QUANTITY _quantity;
  XFEM *_xfem;
};

template<>
InputParameters validParams<XFEMFirstCutPlaneAux>();

#endif //XFEMCUTPLANEAUX_H
