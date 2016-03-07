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
#include "Material.h"

// libMesh includes
#include "libmesh/quadrature.h"


template<>
InputParameters validParams<Material>()
{
  InputParameters params = validParams<MaterialBase>();
  return params;
}


Material::Material(const InputParameters & parameters) :
    MaterialBase(parameters)
{
}


void
Material::computeProperties()
{
  for (_qp = 0; _qp < _qrule->n_points(); ++_qp)
    computeQpProperties();
}


void
Material::computeQpProperties()
{
}
