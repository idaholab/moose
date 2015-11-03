/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "LinearElasticityMaterial.h"

// libmesh includes
#include "libmesh/quadrature.h"

template<>
InputParameters validParams<LinearElasticityMaterial>()
{
  InputParameters params = validParams<Material>();

  params.addCoupledVar("temp", "Coupled Temperature");

  params.set<Real>("thermal_expansion")=1.0;
  params.set<Real>("youngs_modulus")=1.0;
  params.set<Real>("poissons_ratio")=1.0;
  params.set<Real>("t_ref")=300;
  params.set<Real>("lambda")=1.;

  return params;
}

LinearElasticityMaterial::LinearElasticityMaterial(const InputParameters & parameters) :
    Material(parameters),
    _has_temp(isCoupled("temp")),
    _temp(_has_temp ? coupledValue("temp") : _zero),
    _my_thermal_expansion(getParam<Real>("thermal_expansion")),
    _my_youngs_modulus(getParam<Real>("youngs_modulus")),
    _my_poissons_ratio(getParam<Real>("poissons_ratio")),
    _my_t_ref(getParam<Real>("t_ref")),
    _thermal_strain(declareProperty<Real>("thermal_strain")),
    _alpha(declareProperty<Real>("alpha")),
    _youngs_modulus(declareProperty<Real>("youngs_modulus")),
    _poissons_ratio(declareProperty<Real>("poissons_ratio"))
  {}

void
LinearElasticityMaterial::computeProperties()
{
  for (unsigned int qp=0; qp<_qrule->n_points(); qp++)
  {
    if (_has_temp)
      _thermal_strain[qp] = _my_thermal_expansion*(_temp[qp] - _my_t_ref);
    else
      _thermal_strain[qp] = 0;

    _alpha[qp] = _my_thermal_expansion;

    _youngs_modulus[qp]  = _my_youngs_modulus;
    _poissons_ratio[qp] = _my_poissons_ratio;
  }
}

