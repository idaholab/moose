/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef SOLIDMECHTEMPCOUPLEZ
#define SOLIDMECHTEMPCOUPLEZ

#include "SolidMechTempCouple.h"

//Forward Declarations
class SolidMechTempCoupleZ;

template<>
InputParameters validParams<SolidMechTempCoupleZ>();


class SolidMechTempCoupleZ : public SolidMechTempCouple
{
public:

  SolidMechTempCoupleZ(const InputParameters & parameters);
  SolidMechTempCoupleZ(const std::string & deprecated_name, InputParameters parameters); // DEPRECATED CONSTRUCTOR

protected:
  virtual Real computeQpResidual();

  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

};

#endif //SOLIDMECHTEMPCOUPLEZ
