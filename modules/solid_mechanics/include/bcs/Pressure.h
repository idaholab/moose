#ifndef PRESSURE_H
#define PRESSURE_H

#include "BoundaryCondition.h"

//Forward Declarations
class Function;
class Pressure;

template<>
InputParameters validParams<Pressure>();

class Pressure : public BoundaryCondition
{
public:

  Pressure(const std::string & name, InputParameters parameters);

  virtual ~Pressure(){}

protected:

  virtual Real computeQpResidual();

  int _component;

  Real _factor;

  bool _has_function;

  Function * _function;

};

#endif //PRESSURE_H
