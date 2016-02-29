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

// MOOSE includes
#include "DiscreteMaterial.h"

template<>
InputParameters validParams<DiscreteMaterial>()
{
  InputParameters params = validParams<Material>();
  params.set<bool>("allow_cyclic_dependency") = true;
  return params;
}


DiscreteMaterial::DiscreteMaterial(const InputParameters & parameters) :
    Material(parameters)
{
}


void
DiscreteMaterial::computeProperties()
{
}


void
DiscreteMaterial::computeProperties(unsigned int qp)
{
  _qp = qp;
  computeQpProperties();
}
