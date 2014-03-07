#ifndef PRESSURE_H
#define PRESSURE_H

#include "IntegratedBC.h"

//Forward Declarations
class Function;
class Pressure;

template<>
InputParameters validParams<Pressure>();

class Pressure : public IntegratedBC
{
public:

  Pressure(const std::string & name, InputParameters parameters);

  virtual ~Pressure(){}

protected:

  virtual Real computeQpResidual();

  const int _component;

  const Real _factor;

  Function * const _function;

  const PostprocessorValue * const _postprocessor;

};

#endif //PRESSURE_H
