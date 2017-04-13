/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef GENERALIZEDPLANESTRAIN_H
#define GENERALIZEDPLANESTRAIN_H

#include "ScalarKernel.h"

// Forward Declarations
class GeneralizedPlaneStrain;
class GeneralizedPlaneStrainUserObject;

template <>
InputParameters validParams<GeneralizedPlaneStrain>();

class GeneralizedPlaneStrain : public ScalarKernel
{
public:
  GeneralizedPlaneStrain(const InputParameters & parameters);

  virtual void reinit(){};
  virtual void computeResidual();
  virtual void computeJacobian();

  const GeneralizedPlaneStrainUserObject & _gps;
};
#endif // GENERALIZEDPLANESTRAIN_H
