//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef INSTEMPERATURECONVECTEDMESH_H
#define INSTEMPERATURECONVECTEDMESH_H

#include "ADTimeKernel.h"

// Forward Declaration
template <ComputeStage>
class INSTemperatureConvectedMesh;

declareADValidParams(INSTemperatureConvectedMesh);

/**
 * This calculates the time derivative for a coupled variable
 **/
template <ComputeStage compute_stage>
class INSTemperatureConvectedMesh : public ADTimeKernel<compute_stage>
{
public:
  INSTemperatureConvectedMesh(const InputParameters & parameters);

protected:
  virtual ADResidual computeQpResidual() override;

  const ADVariableValue & _disp_x_dot;
  const ADVariableValue & _disp_y_dot;
  const ADVariableValue & _disp_z_dot;

  const ADMaterialProperty(Real) & _rho;
  const ADMaterialProperty(Real) & _cp;

  usingTimeKernelMembers;
};

#endif // INSTEMPERATURECONVECTEDMESH}_H
