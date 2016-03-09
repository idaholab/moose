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

// libMesh includes
#include "libmesh/quadrature.h"

template<>
InputParameters validParams<DiscreteMaterial>()
{
  InputParameters params = validParams<MaterialBase>();
  return params;
}


DiscreteMaterial::DiscreteMaterial(const InputParameters & parameters) :
    MaterialBase(parameters)
{
}


void
DiscreteMaterial::resetProperties()
{
  for (_qp = 0; _qp < _qrule->n_points(); ++_qp)
    resetQpProperties();
}


void
DiscreteMaterial::computeProperties(unsigned int qp)
{
  _qp = qp;
  computeQpProperties();
}
