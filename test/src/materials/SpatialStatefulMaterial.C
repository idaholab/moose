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

#include "SpatialStatefulMaterial.h"

template<>
InputParameters validParams<SpatialStatefulMaterial>()
{
  InputParameters params = validParams<Material>();
  params.addParam<Real>("initial_diffusivity", 0.5, "The Initial Diffusivity");
  return params;
}

SpatialStatefulMaterial::SpatialStatefulMaterial(const std::string & name,
                                 InputParameters parameters)
  :Material(name, parameters),

   // Get a parameter value for the diffusivity
   _initial_diffusivity(getParam<Real>("initial_diffusivity")),

   // Declare that this material is going to have a Real
   // valued property named "diffusivity" that Kernels can use.
   _diffusivity(declareProperty<Real>("diffusivity")),

   // Declare that we are going to have an old value of diffusivity
   // Note: this is _expensive_ and currently means that you can't
   // use adaptivity!  Only do this if you REALLY need it!
   _diffusivity_old(declarePropertyOld<Real>("diffusivity"))
{}

void
SpatialStatefulMaterial::initQpStatefulProperties()
{
  _diffusivity[_qp] = _initial_diffusivity;
}

void
SpatialStatefulMaterial::computeQpProperties()
{
  _diffusivity[_qp] = _diffusivity_old[_qp] + _q_point[_qp](0) + _q_point[_qp](1);
}
