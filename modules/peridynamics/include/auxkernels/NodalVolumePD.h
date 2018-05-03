//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef NODALVOLUMEPD_H
#define NODALVOLUMEPD_H

#include "AuxKernelBasePD.h"

class NodalVolumePD;
class MeshBasePD;

template <>
InputParameters validParams<NodalVolumePD>();

/**
 * Aux Kernel class to output the area/volume of material points
 */
class NodalVolumePD : public AuxKernelBasePD
{
public:
  NodalVolumePD(const InputParameters & parameters);

protected:
  Real computeValue() override;
};

#endif // NODALVOLUMEPD_H
