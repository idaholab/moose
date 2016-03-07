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

  // It doesn't make sense to restrict DiscreteMaterial objects to a block or boundary since the user is
  // responsible for the calculation. However, it is not possible to move Block/BoundaryRestrictable
  // inhertence to Material instead of MaterialBase. This is due to the MaterialPropertyInterface which
  // requires block/boundary ids be supplied on construction. And, the get/setMaterialProperty methods
  // in MaterialBase override methods in MaterialPropertyInterface. Rather than re-factor these
  // interfaces, these parameters are disabled.
  params.suppressParameter<std::vector<SubdomainName> >("block");
  params.suppressParameter<std::vector<BoundaryName> >("boundary");
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
