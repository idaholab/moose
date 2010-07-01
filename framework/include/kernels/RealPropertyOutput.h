#ifndef REALPROPERTYOUTPUT_H
#define REALPROPERTYOUTPUT_H

#include "Kernel.h"


// Forward Declaration
class RealPropertyOutput;

template<>
InputParameters validParams<RealPropertyOutput>();


class RealPropertyOutput : public Kernel
{
public:

  RealPropertyOutput(std::string name, MooseSystem & moose_system, InputParameters parameters);

protected:
  virtual Real computeQpResidual();

  virtual Real computeQpJacobian();

  std::string _prop_name;

  MaterialProperty<Real> & _prop;
};
#endif //REALPROPERTYOUTPUT_H
