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
#include "MaterialBase.h"

// Forward declarations
class DiscreteMaterial;

template<>
InputParameters validParams<DiscreteMaterial>();

/**
 * A material that requires explicit calls to computeProperties.
 *
 * DiscreteMaterial objects may be retrieved via MaterialPropertyInterface::getDiscreteMaterial. There
 * are two methods that must be overridden in your class: resetQpProperties and computeQpProperties.
 */
class DiscreteMaterial :
  public MaterialBase
{
public:
  DiscreteMaterial(const InputParameters & parameters);

  /**
   * A method for (re)computing the properties of a DiscreteMaterial.
   *
   * This is indendeded to be called from other objects, by first calling
   * MaterialPropertyInterface::getDiscreteMaterial and then calling this method on the DiscreateMaterial
   * object returned.
   */
  virtual void computeProperties(unsigned int qp);

  /**
   * Resets the properties at each quadrature point (see resetQpProperties).
   *
   * This method is called internally by MOOSE, you probably don't want to mess with this.
   */
  virtual void resetProperties();

protected:

  /**
   * Resets the properties prior to calculation of traditional materials.
   *
   * This method must be overridden in your class. This is called just prior to the re-calculation of
   * traditional material properties to esnure that the properties are in a proper state for caclulation.
   */
  virtual void resetQpProperties() = 0;

private:
  std::set<std::string> _empty_set;

};
#endif // DISCRETEMATERIAL_H
