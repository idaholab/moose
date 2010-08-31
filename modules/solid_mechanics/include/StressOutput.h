#ifndef STRESSOUTPUT_H
#define STRESSOUTPUT_H

#include "Kernel.h"

//Forward Declarations
class StressOutput;

template<>
InputParameters validParams<StressOutput>();

class StressOutput : public Kernel
{
public:

  StressOutput(std::string name, MooseSystem & moose_system, InputParameters parameters);
    
protected:
  virtual Real computeQpResidual();

  virtual Real computeQpJacobian();

  MaterialProperty<RealTensorValue> & _stress;
  bool _VonMises;
  bool _Hydrostatic;
  int _comp1;
  int _comp2;

private:
  
};
#endif //STRESSOUTPUT_H
