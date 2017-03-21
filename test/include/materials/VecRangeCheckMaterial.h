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
#ifndef VECRANGECHECKMATERIAL_H
#define VECRANGECHECKMATERIAL_H

#include "Material.h"
#include "MaterialProperty.h"

// Forward Declarations
class VecRangeCheckMaterial;

template <>
InputParameters validParams<VecRangeCheckMaterial>();

/**
 * Simple material to test vector parameter range checking.
 */
class VecRangeCheckMaterial : public Material
{
public:
  VecRangeCheckMaterial(const InputParameters & parameters);

protected:
  void computeQpProperties();
};

#endif // VECRANGECHECKMATERIAL_H
