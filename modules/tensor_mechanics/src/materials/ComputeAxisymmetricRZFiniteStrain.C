/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "ComputeAxisymmetricRZFiniteStrain.h"

template<>
InputParameters validParams<ComputeAxisymmetricRZFiniteStrain>()
{
  InputParameters params = validParams<Compute2DFiniteStrain>();
  params.addClassDescription("Compute a strain increment and rotation increment for finite strains under axisymmetric assumptions.");
  return params;
}

ComputeAxisymmetricRZFiniteStrain::ComputeAxisymmetricRZFiniteStrain(const std::string & name,
                                                 InputParameters parameters) :
    Compute2DFiniteStrain(name, parameters)
{
}

Real
ComputeAxisymmetricRZFiniteStrain::computeDeformGradZZ()
{
  if (_assembly.coordSystem() != Moose::COORD_RZ)
    mooseError("The coordinate system must be set to RZ for Axisymmetric simulations.");

  if (_q_point[_qp](0) != 0.0)
    return (*_disp[0])[_qp] / _q_point[_qp](0);

  else
    return 0.0;
}

Real
ComputeAxisymmetricRZFiniteStrain::computeDeformGradZZold()
{
  if (_q_point[_qp](0) != 0.0)
    return (*_disp_old[0])[_qp] / _q_point[_qp](0);

  else
    return 0.0;
}
