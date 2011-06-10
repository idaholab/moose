#ifndef NSPRESSURENEUMANNBC_H
#define NSPRESSURENEUMANNBC_H

#include "IntegratedBC.h"
#include "Material.h"


//Forward Declarations
class NSPressureNeumannBC;

template<>
InputParameters validParams<NSPressureNeumannBC>();

class NSPressureNeumannBC : public IntegratedBC
{
public:

  NSPressureNeumannBC(const std::string & name, InputParameters parameters);

  virtual ~NSPressureNeumannBC(){}

protected:

  /**
   * This is here because materials don't yet work on boundaries!
   * Pressure is now computed as an Aux var
   */
  //Real pressure();
  
  virtual Real computeQpResidual();
  

  VariableValue & _pressure; // Aux Var
  
  int _component;

  MaterialProperty<Real> & _gamma; // Integrated BC, so can use Mat. properties
};

#endif //PRESSURENEUMANNBC_H
