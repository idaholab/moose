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

#ifndef DISCRETEMATERIAL_H
#define DISCRETEMATERIAL_H

// MOOSE includes
#include "Material.h"

// Forward declarations
class DiscreteMaterial;

template<>
InputParameters validParams<DiscreteMaterial>();

/**
 * A material that requires explicit calls to computeProperties.
 *
 * DiscreteMaterial objects may be retriveved via MaterialPropertyInterface::getDiscreteMaterial.
 */
class DiscreteMaterial :
  public Material
{
public:
  DiscreteMaterial(const InputParameters & parameters);

  /**
   * Override the Material::computeProperties so that nothing is computed.
   */
  virtual void computeProperties();

  /**
   * A method for (re)computing the properties of a DiscreteMaterial.
   */
  virtual void computeProperties(unsigned int qp);
};
#endif // DISCRETEMATERIAL_H
