/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "ComputePlaneFiniteStrain.h"

template<>
InputParameters validParams<ComputePlaneFiniteStrain>()
{
  InputParameters params = validParams<Compute2DFiniteStrain>();
  params.addClassDescription("Compute a strain increment and rotation increment for finite strains under axisymmetric assumptions.");
  return params;
}

ComputePlaneFiniteStrain::ComputePlaneFiniteStrain(const InputParameters & parameters) :
    Compute2DFiniteStrain(parameters)
{
}

Real
ComputePlaneFiniteStrain::computeDeformGradZZ()
{
    return 0.0;
}

Real
ComputePlaneFiniteStrain::computeDeformGradZZold()
{
    return 0.0;
}
