#ifndef PLENUMPRESSURE_H
#define PLENUMPRESSURE_H

#include "IntegratedBC.h"

//Forward Declarations
class PlenumPressure;

template<>
InputParameters validParams<PlenumPressure>();

class PlenumPressure : public IntegratedBC
{
public:

  PlenumPressure(const std::string & name, InputParameters parameters);

  virtual ~PlenumPressure(){}

  virtual void initialSetup();

  virtual void residualSetup();

  virtual void timestepSetup();

protected:

  virtual Real computeQpResidual();

  Real _n0; // The initial number of moles of gas.

  const int _component;

  const Real _initial_pressure;

  const Real & _material_input;

  const Real _R;

  const Real & _temperature;

  const Real & _volume;

  const Real _startup_time;

  PostprocessorValue * const _initial_moles;
  PostprocessorValue * const _output;

  const unsigned _refab_needed;
  Real _refab_gas_released;
  const std::vector<Real> _refab_time;
  const std::vector<Real> _refab_pressure;
  const std::vector<Real> _refab_temperature;
  const std::vector<Real> _refab_volume;
  const std::vector<unsigned> _refab_type;
  unsigned _refab_counter;

  Real _my_value;

};

#endif //PLENUMRESSURE_H
