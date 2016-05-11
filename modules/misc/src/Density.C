/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "Density.h"

// libmesh includes
#include "libmesh/quadrature.h"

template<>
InputParameters validParams<Density>()
{
  InputParameters params = validParams<Material>();

  params.addCoupledVar("disp_r", "The r displacement");
  params.addCoupledVar("disp_x", "The x displacement");
  params.addCoupledVar("disp_y", "The y displacement");
  params.addCoupledVar("disp_z", "The z displacement");

  params.addRequiredParam<Real>("density", "Density");

  return params;
}

Density::Density( const InputParameters & parameters) :
  Material(parameters),

  _is_coupled( isCoupled("disp_x") || isCoupled("disp_r") ),
  _is_RZ( isCoupled("disp_r") && isCoupled("disp_z") ),
  _is_SphericalR( isCoupled("disp_r") && !isCoupled("disp_z") ),
  _grad_disp_x( isCoupled("disp_x") ? coupledGradient("disp_x") :
                ( isCoupled("disp_r") ? coupledGradient("disp_r") : _grad_zero ) ),
  _grad_disp_y( isCoupled("disp_y") ? coupledGradient("disp_y") :
                ( isCoupled("disp_z") ? coupledGradient("disp_z") : _grad_zero ) ),
  _grad_disp_z( !_is_RZ && isCoupled("disp_z") ? coupledGradient("disp_z") : _grad_zero ),
  _disp_r( _is_RZ || _is_SphericalR ? coupledValue("disp_r") : _zero ),

  _orig_density(getParam<Real>("density")),
  _density(declareProperty<Real>("density")),
  _density_old(declarePropertyOld<Real>("density"))

{}

void
Density::initStatefulProperties(unsigned n_points)
{
  for (unsigned qp(0); qp < n_points; ++qp)
  {
    _density[qp] = _orig_density;
  }
}

void
Density::computeProperties()
{
  for (unsigned int qp(0); qp < _qrule->n_points(); ++qp)
  {
    Real d(_orig_density);
    if (_is_coupled)
    {
      // rho*V = rho0*V0
      // rho = rho0*V0/V
      // rho = rho0/det(F)
      // rho = rho0/det(grad(u)+I)
      const Real Axx = _grad_disp_x[qp](0) + 1;
      const Real Axy = _grad_disp_x[qp](1);
      const Real Axz = _grad_disp_x[qp](2);
      const Real Ayx = _grad_disp_y[qp](0);
            Real Ayy = _grad_disp_y[qp](1) + 1;
      const Real Ayz = _grad_disp_y[qp](2);
      const Real Azx = _grad_disp_z[qp](0);
      const Real Azy = _grad_disp_z[qp](1);
            Real Azz = _grad_disp_z[qp](2) + 1;
      if (_is_RZ)
      {
        if (_q_point[qp](0)!=0.0)
        {
          Azz = _disp_r[qp]/_q_point[qp](0) + 1;
        }
      }
      else if (_is_SphericalR)
      {
        if (_q_point[qp](0)!=0.0)
        {
          Ayy = Azz = _disp_r[qp]/_q_point[qp](0) + 1;
        }
      }
      const Real detF = Axx*Ayy*Azz + Axy*Ayz*Azx + Axz*Ayx*Azy - Azx*Ayy*Axz - Azy*Ayz*Axx - Azz*Ayx*Axy;
      d /= detF;
    }
    _density[qp] = d;
  }
}
