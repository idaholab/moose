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

#include "ExampleMaterial.h"
#include <cmath>

template<>
InputParameters validParams<ExampleMaterial>()
{
  InputParameters params = validParams<Material>();

  // Vectors for Linear Interpolation
  params.addRequiredCoupledVar("c","variable");
  params.addRequiredParam<Real>("mob", "The mobility value");
  params.addParam<Real>("kappa", 1.0, "The kappa parameter for the vacancy concentration");
  return params;
}

ExampleMaterial::ExampleMaterial(const std::string & name,
                                 InputParameters parameters) :
    Material(name, parameters),
    _eps(declareProperty<Real>("eps")),
    _eps1(declareProperty<Real>("eps1")),
    _u(coupledValue("c")),
    _grad_u(coupledGradient("c")),
    _M(declareProperty<Real>("M")),
    _grad_M(declareProperty<RealGradient>("grad_M")),
    _kappa_c(declareProperty<Real>("kappa_c")),
    _mob(getParam<Real>("mob")),
    _kappa(getParam<Real>("kappa"))
{}

   Real _angle_Mat;
   Real _cos_Mat;
void
ExampleMaterial::computeProperties()
{
  for (unsigned int qp = 0; qp < _qrule->n_points(); ++qp)
  {
    _M[qp] = _mob;
    _grad_M[qp] = 0.0;
    _kappa_c[qp] = _kappa;
    
    // Set value in special situation
    if(_grad_u[qp]*_grad_u[qp] == 0)
    _cos_Mat = 0;
    // Set anisotropic properties value
    else
    _cos_Mat = _grad_u[qp](0)/sqrt(_grad_u[qp]*_grad_u[qp]);
    _angle_Mat =  acos(_cos_Mat);
    _eps[qp]= 0.01*(1+0.04*cos(6*(_angle_Mat-1.57)));
    _eps1[qp]= -0.01*0.04*6*sin(6*(_angle_Mat-1.57));
   }
}


