/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef SOLIDMECHTEMPCOUPLEX
#define SOLIDMECHTEMPCOUPLEX

#include "SolidMechTempCouple.h"

//Forward Declarations
class SolidMechTempCoupleX;

template<>
InputParameters validParams<SolidMechTempCoupleX>();

class SolidMechTempCoupleX : public SolidMechTempCouple
{
public:

  SolidMechTempCoupleX(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual();

  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

};
#endif //SOLIDMECHTEMPCOUPLEX
