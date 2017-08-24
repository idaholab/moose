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
#ifndef SUBDOMAINCONSTANTMATERIAL_H
#define SUBDOMAINCONSTANTMATERIAL_H

#include "Material.h"

class SubdomainConstantMaterial;

template <>
InputParameters validParams<SubdomainConstantMaterial>();

/**
 * Simple material with subdomain-wise constant properties.
 */
class SubdomainConstantMaterial : public Material
{
public:
  SubdomainConstantMaterial(const InputParameters & parameters);

protected:
  virtual void computeSubdomainProperties();

  const MaterialPropertyName & _mat_prop_name;
  MaterialProperty<Real> & _mat_prop;
  std::map<SubdomainID, Real> _mapped_values;
};

#endif // SUBDOMAINCONSTANTMATERIAL_H
