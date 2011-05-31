#ifndef PRESSURENEUMANNBC_H
#define PRESSURENEUMANNBC_H

#include "IntegratedBC.h"
#include "Material.h"


//Forward Declarations
class PressureNeumannBC;

template<>
InputParameters validParams<PressureNeumannBC>();

class PressureNeumannBC : public IntegratedBC
{
public:

  PressureNeumannBC(const std::string & name, InputParameters parameters);

  virtual ~PressureNeumannBC(){}

protected:

  /**
   * This is here because materials don't yet work on boundaries!
   * Pressure is now computed as an Aux var
   */
  //Real pressure();
  
  virtual Real computeQpResidual();
  

  //VariableValue & _p;

  //VariableValue & _pe;

  //VariableValue & _pu;
  //VariableValue & _pv;
  //VariableValue & _pw;

  VariableValue & _pressure; // Aux Var
  
  int _component;

  MaterialProperty<Real> & _gamma; // Integrated BC, so can use Mat. properties
};

#endif //PRESSURENEUMANNBC_H
