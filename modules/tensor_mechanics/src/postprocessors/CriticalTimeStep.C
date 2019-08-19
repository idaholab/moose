//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CriticalTimeStep.h"

registerMooseObject("TensorMechanicsApp", CriticalTimeStep);

template <>
InputParameters
validParams<CriticalTimeStep>()
{
  InputParameters params = validParams<ElementPostprocessor>();
  params.addClassDescription("Computes and reports the critical time step for the explicit solver.");
  return params;
}

CriticalTimeStep::CriticalTimeStep(const InputParameters & parameters)
  : ElementPostprocessor(parameters),
  GuaranteeProvider(this),
  _effective_stiffness(getMaterialPropertyByName<Real>("effective_stiffness"))
{
}

void
CriticalTimeStep::initialize()
{
  _total_size = 1e10;
  _elems = 0;
}

void
CriticalTimeStep::execute()
{
  _total_size = std::min(_current_elem->hmin(), _total_size);
  _elems++;
}

Real
CriticalTimeStep::getValue()
{
  gatherSum(_total_size);
  gatherSum(_elems);

//  std::cout <<  << std::endl;

  /*Real lame_1 = _elasticity_tensor[0](0,0,1,1);
  Real lame_2 = _elasticity_tensor[0](0,1,0,1);
  Real dens = _mat_dens[0];

  Real elas_mod = lame_2*(3*lame_1+2*lame_2)/(lame_1+lame_2);
  Real poiss_rat = lame_1/(2*(lame_1+lame_2));

  Real ele_c = std::sqrt((elas_mod*(1-poiss_rat))/((1+poiss_rat)*(1-2*poiss_rat) * (dens)));*/

  return _total_size/_effective_stiffness[0];

  std::cout<< _total_size/_effective_stiffness[0] << std::endl;
}

void
CriticalTimeStep::threadJoin(const UserObject & y)
{
  const CriticalTimeStep & pps = static_cast<const CriticalTimeStep &>(y);
  _total_size += pps._total_size;
  _elems += pps._elems;
}
