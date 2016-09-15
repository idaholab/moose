/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef GENERALIZEDPLANESTRAINDIAG_H
#define GENERALIZEDPLANESTRAINDIAG_H

#include "ScalarKernel.h"
#include "GeneralizedPlaneStrainUO.h"

//Forward Declarations
class GeneralizedPlaneStrainDiag;

template<>
InputParameters validParams<GeneralizedPlaneStrainDiag>();

class GeneralizedPlaneStrainDiag : public ScalarKernel
{
public:
  GeneralizedPlaneStrainDiag(const InputParameters & parameters);

  virtual void reinit(){};
  virtual void computeResidual();
  virtual void computeJacobian();

private:
  const GeneralizedPlaneStrainUO & _gps_uo;
};
#endif //GENERALIZEDPLANESTRAINDIAG_H
