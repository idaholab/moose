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

#ifndef OLDMATERIALAUX_H
#define OLDMATERIALAUX_H

// MOOSE includes
#include "AuxKernel.h"

// Forward declarations
class OldMaterialAux;

template <>
InputParameters validParams<OldMaterialAux>();

/**
 * Test class for testing functionality of getMaterialPropertyOld/Older for AuxKernels
 */
class OldMaterialAux : public AuxKernel
{
public:
  OldMaterialAux(const InputParameters & parameters);
  virtual ~OldMaterialAux() {}

protected:
  virtual Real computeValue();

private:
  const MaterialProperty<Real> & _old;
  const MaterialProperty<Real> & _older;
};

#endif // OLDMATERIALAUX_H
