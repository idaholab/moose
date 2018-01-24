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

#ifndef MATERIALPROPBLOCKAUX_H
#define MATERIALPROPBLOCKAUX_H

#include "AuxKernel.h"

// Forward Declarations
class MaterialPropertyBlockAux;

template <>
InputParameters validParams<MaterialPropertyBlockAux>();

/**
 * This AuxKernel sets an elemental aux field to one on blocks where
 * a material property is defined and zero elsewhere. This class
 * throws an error if used on a Lagrange basis.
 */
class MaterialPropertyBlockAux : public AuxKernel
{
public:
  MaterialPropertyBlockAux(const InputParameters & params);

protected:
  virtual void subdomainSetup() override;
  virtual Real computeValue() override;

  const MaterialPropertyName & _mat_prop_name;
  bool _has_prop;
};

#endif // MATERIALPROPBLOCKAUX_H
