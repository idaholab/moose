#ifndef PRESSURENEUMANNBC_H
#define PRESSURENEUMANNBC_H

#include "BoundaryCondition.h"
#include "Material.h"


//Forward Declarations
class PressureNeumannBC;

template<>
InputParameters validParams<PressureNeumannBC>();

class PressureNeumannBC : public BoundaryCondition
{
public:

  PressureNeumannBC(std::string name, MooseSystem & moose_system, InputParameters parameters);

  virtual ~PressureNeumannBC(){}

protected:

  /**
   * This is here because materials don't yet work on boundaries!
   */
  Real pressure();
  
  virtual Real computeQpResidual();
  

  MooseArray<Real> & _p;

  MooseArray<Real> & _pe;

  MooseArray<Real> & _pu;
  MooseArray<Real> & _pv;
  MooseArray<Real> & _pw;

  int _component;
};

#endif //PRESSURENEUMANNBC_H
