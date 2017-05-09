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

#ifndef TESTSAMPLERMATERIAL_H
#define TESTSAMPLERMATERIAL_H

#include "GenericConstantMaterial.h"
#include "SamplerInterface.h"
#include "Sampler.h"

// Forward Declarations
class TestSamplerMaterial;

template <>
InputParameters validParams<TestSamplerMaterial>();

/**
 * This material automatically declares as material properties whatever is passed to it
 * through the parameters 'prop_names' and uses the perturbed factor times the values
 * from 'prop_values' as the values for those properties.
 */
class TestSamplerMaterial : public GenericConstantMaterial, public SamplerInterface
{
public:
  TestSamplerMaterial(const InputParameters & parameters);

protected:
  /// Object of sampler
  Sampler & _sampler;
};

#endif // TESTSAMPLERMATERIAL_H
