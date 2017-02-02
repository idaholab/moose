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
#ifndef ERROR_MATERIAL_H
#define ERROR_MATERIAL_H

#include "Material.h"

class ErrorMaterial;

template<>
InputParameters validParams<ErrorMaterial>();

/**
 * ErrorMaterial throws a MooseException when certain conditions are
 * met.
 */
class ErrorMaterial : public Material
{
public:
  ErrorMaterial(const InputParameters & parameters);

protected:
  virtual void computeQpProperties() override;
};

#endif
