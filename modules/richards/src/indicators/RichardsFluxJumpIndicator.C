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

#include "RichardsFluxJumpIndicator.h"

template<>
InputParameters validParams<RichardsFluxJumpIndicator>()
{
  InputParameters params = validParams<JumpIndicator>();
  params.addClassDescription("Indicator which calculates jumps in Richards fluxes over elements.");
  return params;
}


RichardsFluxJumpIndicator::RichardsFluxJumpIndicator(const std::string & name, InputParameters parameters) :
    JumpIndicator(name, parameters),
    _density(getMaterialProperty<Real>("density")),
    _rel_perm(getMaterialProperty<Real>("rel_perm")),
    _dens0(getMaterialProperty<Real>("dens0")),
    _gravity(getMaterialProperty<RealVectorValue>("gravity")),
    _permeability(getMaterialProperty<RealTensorValue>("permeability")),

    _density_n(getNeighborMaterialProperty<Real>("density")),
    _rel_perm_n(getNeighborMaterialProperty<Real>("rel_perm")),
    _dens0_n(getNeighborMaterialProperty<Real>("dens0")),
    _gravity_n(getNeighborMaterialProperty<RealVectorValue>("gravity")),
    _permeability_n(getNeighborMaterialProperty<RealTensorValue>("permeability"))
{
}


Real
RichardsFluxJumpIndicator::computeQpIntegral()
{
  RealVectorValue gra = _density[_qp]*_rel_perm[_qp]*(_permeability[_qp]*(_grad_u[_qp] - _dens0[_qp]*_gravity[_qp]));
  RealVectorValue gra_n = _density_n[_qp]*_rel_perm_n[_qp]*(_permeability_n[_qp]*(_grad_u_neighbor[_qp] - _dens0_n[_qp]*_gravity_n[_qp]));

  Real jump = (gra - gra_n)*_normals[_qp];

  return jump*jump;
}

