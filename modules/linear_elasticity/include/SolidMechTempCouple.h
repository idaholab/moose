#ifndef SOLIDMECHTEMPCOUPLE_H
#define SOLIDMECHTEMPCOUPLE_H

#include "SolidMech.h"

//libMesh includes
#include "tensor_value.h"

//Forward Declarations
class SolidMechTempCouple;

template<>
InputParameters validParams<SolidMechTempCouple>();

class SolidMechTempCouple : public SolidMech
{
public:

  SolidMechTempCouple(const std::string & name, MooseSystem & moose_system, InputParameters parameters);
  
  void recomputeCouplingConstants();
  
protected:
  unsigned int _temp_var;
  
  MaterialProperty<Real> & _thermal_strain;
  MaterialProperty<Real> & _alpha;

  Real _c4;
};
 

#endif //SOLIDMECHTEMPCOUPLE_H
