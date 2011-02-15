#ifndef PLENUMPRESSURE_H
#define PLENUMPRESSURE_H

#include "BoundaryCondition.h"

//Forward Declarations
class PlenumPressure;

template<>
InputParameters validParams<PlenumPressure>();

class PlenumPressure : public BoundaryCondition
{
public:

  PlenumPressure(const std::string & name, InputParameters parameters);

  virtual ~PlenumPressure(){}

  virtual void setup();

protected:

  virtual Real computeQpResidual();

  bool _initialized;

  Real _n0; // The initial number of moles of gas.

  const int _component;

  const Real _initial_pressure;

  const Real & _material_input;

  const Real _R;

  const Real & _temperature;

  const Real & _volume;

  const Real _startup_time;

  PostprocessorValue * const _initial_He;
  PostprocessorValue * const _output;

  Real _my_value;

};

#endif //PLENUMRESSURE_H
