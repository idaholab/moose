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

#include "EnhancedDGMatDiffusion.h"

template<>
InputParameters validParams<EnhancedDGMatDiffusion>()
{
  InputParameters params = validParams<DGKernel>();

  params.addRequiredParam<Real>("sigma", "sigma");
  params.addRequiredParam<Real>("epsilon", "epsilon");
  params.addRequiredParam<std::string>("diffusivity", "diffusivity");
  params.addParam<Real>("limit", 10.0, "limiting factor");
  params.addParam<Real>("x", 0.0, "Component of velocity in the x direction");
  params.addParam<Real>("y", 0.0, "Component of velocity in the y direction");
  params.addParam<Real>("z", 0.0, "Component of velocity in the z direction");
  params.addParam<Real>("adaptive", 0.0, "Use of Adaptive DG method?");
  return params;
}


EnhancedDGMatDiffusion::EnhancedDGMatDiffusion(const std::string & name, InputParameters parameters)
  :DGKernel(name, parameters),
   _epsilon(getParam<Real>("epsilon")),
   _sigma(getParam<Real>("sigma")),
   _limiting_factor(getParam<Real>("limit")),
   _x(getParam<Real>("x")),
   _y(getParam<Real>("y")),
   _z(getParam<Real>("z")),
   _adaptive(getParam<Real>("adaptive")),
   _prop_name(getParam<std::string>("diffusivity")),
   _diff(getMaterialProperty<Real>(_prop_name)),
   _diff_neighbor(getNeighborMaterialProperty<Real>(_prop_name))
{
  _velocity(0) = _x;
  _velocity(1) = _y;
  _velocity(2) = _z;

  if ((_x == 0) && (_y == 0) && (_z == 0))
  {
    _conv = 0.0;
  }
  else
  {
    _conv = 1.0;
  }
}

Real
EnhancedDGMatDiffusion::computeQpResidual(Moose::DGResidualType type)
{
  Real r = 0;

  const unsigned int elem_b_order = static_cast<unsigned int> (_var.getOrder());
  const double h_elem = _current_elem->volume()/_current_side_elem->volume() * 1./pow(elem_b_order, 2.);


  Real _omega = 0.5;
  Real _omega_neighbor = 0.5;

  if (((_diff[_qp] <= _diff_neighbor[_qp] * _limiting_factor) && (_diff[_qp] * _limiting_factor >= _diff_neighbor[_qp])) || (_conv == 0.0))
  {
    switch (type)
    {
    case Moose::Element:
      r += - _omega * _diff[_qp] * _grad_u[_qp] * _normals[_qp] *_test[_i][_qp] + _omega * _epsilon * _diff[_qp] * _grad_test[_i][_qp] * _normals[_qp] * _u[_qp];
      r += _sigma / h_elem * _u[_qp] * _test[_i][_qp];

      r += - _omega_neighbor * _diff_neighbor[_qp] * _grad_u_neighbor[_qp] * _normals[_qp] * _test[_i][_qp] - _omega * _epsilon * _diff[_qp] * _grad_test[_i][_qp] * _normals[_qp] * _u_neighbor[_qp];
      r += - _sigma / h_elem * _u_neighbor[_qp] * _test[_i][_qp];
      break;
      
    case Moose::Neighbor:
      r += _omega_neighbor * _diff_neighbor[_qp] * _grad_u_neighbor[_qp] * _normals[_qp] * _test_neighbor[_i][_qp] - _omega_neighbor * _epsilon * _diff_neighbor[_qp] * _grad_test_neighbor[_i][_qp] * _normals[_qp] * _u_neighbor[_qp];
      r += _sigma / h_elem * _u_neighbor[_qp] * _test_neighbor[_i][_qp];

      r += _omega * _diff[_qp] * _grad_u[_qp] * _normals[_qp] * _test_neighbor[_i][_qp] + _omega_neighbor * _epsilon * _diff_neighbor[_qp] * _grad_test_neighbor[_i][_qp] * _normals[_qp] * _u[_qp];
      r += - _sigma / h_elem * _u[_qp] * _test_neighbor[_i][_qp];
      break;
    }
    return r;
  }
  else
  {
    Real _theta = 0.0;
    if (_adaptive != 0.0)
    {
      if (_diff_neighbor[_qp] == 0.0)
      {
        _theta = 0.0;
      }
      else
      {
        _theta = _diff[_qp]/_diff_neighbor[_qp];
        if (_theta > 1.0)
        {
          _theta = _diff_neighbor[_qp]/_diff[_qp];
        }
      }
    }
    
    
    
    
    if (_velocity*_normals[_qp] >= 0)
    {
      switch (type)
      {
      case Moose::Element:
        r += _sigma / h_elem * _diff[_qp] *_u[_qp] * _test[_i][_qp];
        r += (1.0 - _theta) * (- _diff[_qp] * _grad_u[_qp] * _normals[_qp] * _test[_i][_qp] + _epsilon * _diff[_qp] * _grad_test[_i][_qp] * _normals[_qp] * _u[_qp]);

        r += - _sigma / h_elem * _diff[_qp] * _u_neighbor[_qp] * _test[_i][_qp];
        r += (1.0 - _theta) * (- _epsilon * _diff[_qp] * _grad_test[_i][_qp] * _normals[_qp] * _u_neighbor[_qp]);

        r += _theta * (- _omega * _diff[_qp] * _grad_u[_qp] * _normals[_qp] * _test[_i][_qp] + _epsilon * _omega * _diff[_qp] * _grad_test[_i][_qp] * _normals[_qp] * _u[_qp]);
        r += _theta * (_omega_neighbor * _diff_neighbor[_qp] * _grad_u_neighbor[_qp] * _normals[_qp] * _test[_i][_qp] + _epsilon * _omega * _diff[_qp] * _grad_test[_i][_qp] * _normals[_qp] * _u_neighbor[_qp]);
        break;

      case Moose::Neighbor:
        r += _sigma / h_elem * _diff[_qp] *  _u_neighbor[_qp] * _test_neighbor[_i][_qp];

        r += - _sigma / h_elem * _diff[_qp] * _u[_qp] * _test_neighbor[_i][_qp];
        r += (1.0 - _theta) * _diff[_qp] * _grad_u[_qp] * _normals[_qp] * _test_neighbor[_i][_qp];

        r += _theta * (_omega * _diff[_qp] * _grad_u[_qp] * _normals[_qp] * _test_neighbor[_i][_qp] + _epsilon * _omega_neighbor * _diff_neighbor[_qp] * _grad_test_neighbor[_i][_qp] * _normals[_qp] * _u[_qp]);
        r += _theta * (- _omega_neighbor * _diff_neighbor[_qp] * _grad_u_neighbor[_qp] * _normals[_qp] * _test_neighbor[_i][_qp] + _epsilon * _omega_neighbor * _diff_neighbor[_qp] * _grad_test_neighbor[_i][_qp] * _normals[_qp] * _u_neighbor[_qp]);
        break;
      }
    }
    else
    {
      switch (type)
      {
      case Moose::Element:
        r +=  _sigma / h_elem * _diff_neighbor[_qp] * _u[_qp] * _test[_i][_qp];

        r += - _sigma / h_elem * _diff_neighbor[_qp] * _u_neighbor[_qp] * _test[_i][_qp];
        r += (1.0 - _theta) * (- _diff_neighbor[_qp] * _grad_u_neighbor[_qp] * _normals[_qp] * _test[_i][_qp]);

        r += _theta * (- _omega * _diff[_qp] * _grad_u[_qp] * _normals[_qp] * _test[_i][_qp] + _epsilon * _omega * _diff[_qp] * _grad_test[_i][_qp] * _normals[_qp] * _u[_qp]);
        r += _theta * (- _omega_neighbor * _diff_neighbor[_qp] * _grad_u_neighbor[_qp] * _normals[_qp] * _test[_i][_qp] - _epsilon * _omega * _diff[_qp] * _grad_test[_i][_qp] * _normals[_qp] * _u_neighbor[_qp]);
        break;

      case Moose::Neighbor:
        r += _sigma / h_elem * _diff_neighbor[_qp] * _u_neighbor[_qp] * _test_neighbor[_i][_qp];
        r += (1.0 - _theta) * (_diff_neighbor[_qp] * _grad_u_neighbor[_qp] * _normals[_qp] * _test_neighbor[_i][_qp] - _epsilon * _diff_neighbor[_qp] * _grad_test_neighbor[_i][_qp] * _normals[_qp] * _u_neighbor[_qp]);

        r += - _sigma / h_elem * _diff_neighbor[_qp] * _u[_qp] * _test_neighbor[_i][_qp];
        r += (1.0 - _theta) * (_epsilon * _diff_neighbor[_qp] * _grad_test_neighbor[_i][_qp] * _normals[_qp] * _u[_qp]);

        r += _theta * (_omega * _diff[_qp] * _grad_u[_qp] * _normals[_qp] * _test_neighbor[_i][_qp] + _epsilon * _omega_neighbor * _diff_neighbor[_qp] * _grad_test_neighbor[_i][_qp] * _normals[_qp] * _u[_qp]);
        r += _theta * (_omega_neighbor * _diff_neighbor[_qp] * _grad_u_neighbor[_qp] * _normals[_qp] * _test_neighbor[_i][_qp] - _epsilon * _omega_neighbor * _diff_neighbor[_qp] * _grad_test_neighbor[_i][_qp] * _normals[_qp] * _u_neighbor[_qp]);
        break;
      }
    }
    return r;
  }
}

Real
EnhancedDGMatDiffusion::computeQpJacobian(Moose::DGJacobianType type)
{
  Real r = 0;

  const unsigned int elem_b_order = static_cast<unsigned int> (_var.getOrder());
  const double h_elem = _current_elem->volume()/_current_side_elem->volume() * 1./pow(elem_b_order, 2.);

  Real _omega = 0.5;
  Real _omega_neighbor = 0.5;

  if (((_diff[_qp] <= _diff_neighbor[_qp] * _limiting_factor) && (_diff[_qp] * _limiting_factor >= _diff_neighbor[_qp])) || (_conv == 0.0))
  {
    switch (type)
    {
    case Moose::ElementElement:
      r += - _omega * _diff[_qp] * _grad_test[_j][_qp] * _normals[_qp] *_test[_i][_qp] + _omega * _epsilon * _diff[_qp] * _grad_test[_i][_qp] * _normals[_qp] * _test[_j][_qp];
      r += _sigma / h_elem * _test[_j][_qp] * _test[_i][_qp];
      break;
      
    case Moose::ElementNeighbor:
      r += - _omega_neighbor * _diff_neighbor[_qp] * _grad_test_neighbor[_j][_qp] * _normals[_qp] * _test[_i][_qp] - _omega * _epsilon * _diff[_qp] * _grad_test[_i][_qp] * _normals[_qp] * _test_neighbor[_j][_qp];
      r += - _sigma / h_elem * _test_neighbor[_j][_qp] * _test[_i][_qp];
      break;
      
    case Moose::NeighborElement:
      r += _omega * _diff[_qp] * _grad_test[_j][_qp] * _normals[_qp] * _test_neighbor[_i][_qp] + _omega_neighbor * _epsilon * _diff_neighbor[_qp] * _grad_test_neighbor[_i][_qp] * _normals[_qp] * _test[_j][_qp];
      r += - _sigma / h_elem * _test[_j][_qp] * _test_neighbor[_i][_qp];
      break;
      
    case Moose::NeighborNeighbor:
      r += _omega_neighbor * _diff_neighbor[_qp] * _grad_test_neighbor[_j][_qp] * _normals[_qp] * _test_neighbor[_i][_qp] - _omega_neighbor * _epsilon * _diff_neighbor[_qp] * _grad_test_neighbor[_i][_qp] * _normals[_qp] * _test_neighbor[_j][_qp];
      r += _sigma / h_elem * _test_neighbor[_j][_qp] * _test_neighbor[_i][_qp];
      break;
    }
    return r;
  }
  else
  {
    Real _theta = 0.0;
    if (_adaptive != 0.0)
    {
      if (_diff_neighbor[_qp] == 0.0)
      {
        _theta = 0.0;
      }
      else
      {
        _theta = _diff[_qp]/_diff_neighbor[_qp];
        if (_theta > 1.0)
        {
          _theta = _diff_neighbor[_qp]/_diff[_qp];
        }
      }
    }
    
    
    if (_velocity*_normals[_qp] >= 0)
    {
      switch (type)
      {
      case Moose::ElementElement:
        r += _sigma / h_elem * _diff[_qp] *_test[_j][_qp] * _test[_i][_qp];
        r += (1.0 - _theta) * (- _diff[_qp] * _grad_test[_j][_qp] * _normals[_qp] * _test[_i][_qp] + _epsilon * _diff[_qp] * _grad_test[_i][_qp] * _normals[_qp] * _test[_j][_qp]);

        r += _theta * (- _omega * _diff[_qp] * _grad_test[_j][_qp] * _normals[_qp] * _test[_i][_qp] + _epsilon * _omega * _diff[_qp] * _grad_test[_i][_qp] * _normals[_qp] * _test[_j][_qp]);
        break;

      case Moose::ElementNeighbor:
        r += - _sigma / h_elem * _diff[_qp] * _test_neighbor[_j][_qp] * _test[_i][_qp];
        r += (1.0 - _theta) * (- _epsilon * _diff[_qp] * _grad_test[_i][_qp] * _normals[_qp] * _test_neighbor[_j][_qp]);
        
        r += _theta * (- _omega_neighbor * _diff_neighbor[_qp] * _grad_test_neighbor[_j][_qp] * _normals[_qp] * _test[_i][_qp] - _epsilon * _omega * _diff[_qp] * _grad_test[_i][_qp] * _normals[_qp] * _test_neighbor[_j][_qp]);
        break;

      case Moose::NeighborElement:
        r += - _sigma / h_elem * _diff[_qp] * _test[_j][_qp] * _test_neighbor[_i][_qp];
        r += (1.0 - _theta) * (_diff[_qp] * _grad_test[_j][_qp] * _normals[_qp] * _test_neighbor[_i][_qp]);

        r += _theta * (_omega * _diff[_qp] * _grad_test[_j][_qp] * _normals[_qp] * _test_neighbor[_i][_qp] + _epsilon * _omega_neighbor * _diff_neighbor[_qp] * _grad_test_neighbor[_i][_qp] * _normals[_qp] * _test[_j][_qp]);
        break;

      case Moose::NeighborNeighbor:
        r += _sigma / h_elem * _diff[_qp] *  _test_neighbor[_j][_qp] * _test_neighbor[_i][_qp];
        
        r += _theta * (_omega_neighbor * _diff_neighbor[_qp] * _grad_test_neighbor[_j][_qp] * _normals[_qp] * _test_neighbor[_i][_qp] - _epsilon * _omega_neighbor * _diff_neighbor[_qp] * _grad_test_neighbor[_i][_qp] * _normals[_qp] * _test_neighbor[_j][_qp]);
        break;
      }
    }
    else
    {
      switch (type)
      {
      case Moose::ElementElement:
        r +=  _sigma / h_elem * _diff_neighbor[_qp] * _test[_j][_qp] * _test[_i][_qp];

        r += _theta * (- _omega * _diff[_qp] * _grad_test[_j][_qp] * _normals[_qp] * _test[_i][_qp] + _epsilon * _omega * _diff[_qp] * _grad_test[_i][_qp] * _normals[_qp] * _test[_j][_qp]);
        break;

      case Moose::ElementNeighbor:
        r += - _sigma / h_elem * _diff_neighbor[_qp] * _test_neighbor[_j][_qp] * _test[_i][_qp];
        r += (1.0 - _theta) * (- _diff_neighbor[_qp] * _grad_test_neighbor[_j][_qp] * _normals[_qp] * _test[_i][_qp]);
        
        r += _theta * (- _omega_neighbor * _diff_neighbor[_qp] * _grad_test_neighbor[_j][_qp] * _normals[_qp] * _test[_i][_qp] - _epsilon * _omega * _diff[_qp] * _grad_test[_i][_qp] * _normals[_qp] * _test_neighbor[_j][_qp]);
        break;

      case Moose::NeighborElement:
        r += - _sigma / h_elem * _diff_neighbor[_qp] * _test[_j][_qp] * _test_neighbor[_i][_qp];
        r += (1.0 - _theta) * (_epsilon * _diff_neighbor[_qp] * _grad_test_neighbor[_i][_qp] * _normals[_qp] * _test[_j][_qp]);

        r += _theta * (_omega * _diff[_qp] * _grad_test[_j][_qp] * _normals[_qp] * _test_neighbor[_i][_qp] + _epsilon * _omega_neighbor * _diff_neighbor[_qp] * _grad_test_neighbor[_i][_qp] * _normals[_qp] * _test[_j][_qp]);
        break;

      case Moose::NeighborNeighbor:
        r += _sigma / h_elem * _diff_neighbor[_qp] * _test_neighbor[_j][_qp] * _test_neighbor[_i][_qp];
        r += (1.0 - _theta) * (_diff_neighbor[_qp] * _grad_test_neighbor[_j][_qp] * _normals[_qp] * _test_neighbor[_i][_qp] - _epsilon * _diff_neighbor[_qp] * _grad_test_neighbor[_i][_qp] * _normals[_qp] * _test_neighbor[_j][_qp]);
        
        r += _theta * (_omega_neighbor * _diff_neighbor[_qp] * _grad_test_neighbor[_j][_qp] * _normals[_qp] * _test_neighbor[_i][_qp] - _epsilon * _omega_neighbor * _diff_neighbor[_qp] * _grad_test_neighbor[_i][_qp] * _normals[_qp] * _test_neighbor[_j][_qp]);
        break;
      }
    }
    return r;
  }
}
