/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "ComputePlaneSmallStrain.h"

template<>
InputParameters validParams<ComputePlaneSmallStrain>()
{
  InputParameters params = validParams<Compute2DSmallStrain>();
  params.addClassDescription("Compute a small strain under traditional plane strain assumptions where the out of plane strain is zero.");
  return params;
}

ComputePlaneSmallStrain::ComputePlaneSmallStrain(const InputParameters & parameters) :
    Compute2DSmallStrain(parameters)
{
}

Real
ComputePlaneSmallStrain::computeStrainZZ()
{
  return 0.0;
}

