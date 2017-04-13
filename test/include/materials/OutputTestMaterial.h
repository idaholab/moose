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

#ifndef OUTPUTTESTMATERIAL_H
#define OUTPUTTESTMATERIAL_H

// MOOSE includes
#include "Material.h"

// Forward declarations
class OutputTestMaterial;

template <>
InputParameters validParams<OutputTestMaterial>();

/**
 *
 */
class OutputTestMaterial : public Material
{
public:
  /**
   * Class constructor
   * @param prop_name
   */
  OutputTestMaterial(const InputParameters & parameters);

  // Used for testing if hidden compiler warning shows up
  virtual void computeProperties() { Material::computeProperties(); }

  /**
   * Class destructor
   */
  virtual ~OutputTestMaterial();

protected:
  virtual void computeQpProperties();

  MaterialProperty<Real> & _real_property;
  MaterialProperty<RealVectorValue> & _vector_property;
  MaterialProperty<RealTensorValue> & _tensor_property;
  Real _factor;
  const VariableValue & _variable;
};

#endif // OUTPUTTESTMATERIAL_H
