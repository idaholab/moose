/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef CYLINDRICALRANKTWOAUX_H
#define CYLINDRICALRANKTWOAUX_H

#include "AuxKernel.h"
#include "RankTwoTensor.h"

/**
 * CylindricalRankTwoAux is designed to take the data in the CylindricalRankTwoTensor material
 * property, for example stress or strain, and output the value for the
 * supplied indices in cylindrical coordinates, where the cylindrical plane axis is
 * along the z-axis and the center point in the x-y plan is defined by by center_point.
 */
class CylindricalRankTwoAux;

template <>
InputParameters validParams<CylindricalRankTwoAux>();

class CylindricalRankTwoAux : public AuxKernel
{
public:
  CylindricalRankTwoAux(const InputParameters & parameters);
  virtual ~CylindricalRankTwoAux() {}

protected:
  virtual Real computeValue();
  const MaterialProperty<RankTwoTensor> & _tensor;
  const unsigned int _i;
  const unsigned int _j;
  const Point _center_point;
};

#endif // CYLINDRICALRANKTWOAUX_H
