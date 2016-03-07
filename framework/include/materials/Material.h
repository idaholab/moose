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

#ifndef MATERIAL_H
#define MATERIAL_H

#include "MaterialBase.h"

// Forward declarations
class Material;

template<>
InputParameters validParams<Material>();

/**
 * Holds material properties that are assigned to blocks and computed by MOOSE.
 */
class Material : public MaterialBase
{
public:

  Material(const InputParameters & parameters);

  /**
   * Performs the quadrature point loop, calling computeQpProperties
   */
  virtual void computeProperties();

protected:
  /**
   * Users must override this method.
   */
  virtual void computeQpProperties();
};


#endif //MATERIAL_H
