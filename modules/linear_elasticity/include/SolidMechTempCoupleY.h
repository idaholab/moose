/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef SOLIDMECHTEMPCOUPLEY
#define SOLIDMECHTEMPCOUPLEY

#include "SolidMechTempCouple.h"

//Forward Declarations
class SolidMechTempCoupleY;

template<>
InputParameters validParams<SolidMechTempCoupleY>();


class SolidMechTempCoupleY : public SolidMechTempCouple
{
public:

  SolidMechTempCoupleY(const InputParameters & parameters);
  SolidMechTempCoupleY(const std::string & deprecated_name, InputParameters parameters); // DEPRECATED CONSTRUCTOR

protected:
  virtual Real computeQpResidual();

  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

};

#endif //SOLIDMECHTEMPCOUPLEY
