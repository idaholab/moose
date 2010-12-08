#ifndef PRESSUREBC_H
#define PRESSUREBC_H

#include "BoundaryCondition.h"

//Forward Declarations
class Function;
class PressureBC;

template<>
InputParameters validParams<PressureBC>();

class PressureBC : public BoundaryCondition
{
public:

  PressureBC(const std::string & name, InputParameters parameters);

  virtual ~PressureBC(){}

protected:

  virtual Real computeQpResidual();

  int _component;

  Real _factor;

  bool _has_function;

  Function * _function;

};

#endif //PRESSUREBC_H
