/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "SolidMechTempCouple.h"
#include "MooseMesh.h"

template<>
InputParameters validParams<SolidMechTempCouple>()
{
  InputParameters params = validParams<SolidMech>();
  params.addRequiredCoupledVar("temp", "Coupled Temperature");
  return params;
}

SolidMechTempCouple::SolidMechTempCouple(const InputParameters & parameters)
  :SolidMech(parameters),
    _temp_var(coupled("temp")),
    _thermal_strain(getMaterialProperty<Real>("thermal_strain")),
   _alpha(getMaterialProperty<Real>("alpha")),
   _mesh_dimension(_mesh.dimension())
{}

void
SolidMechTempCouple::subdomainSetup()
{
  if (_constant_properties)
  {
    _qp = 0;
    recomputeCouplingConstants();
  }
}

void
SolidMechTempCouple::recomputeCouplingConstants()
{
  recomputeConstants();

  _c4 = _E/(1.-_nu);

  if (_mesh_dimension == 3)
    _c4 = _E/(1.-2.*_nu);
}

