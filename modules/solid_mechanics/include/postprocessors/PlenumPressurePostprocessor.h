#ifndef PLENUMPRESSUREPOSTPROCESSOR_H
#define PLENUMPRESSUREPOSTPROCESSOR_H

#include "GeneralPostprocessor.h"

class PlenumPressurePostprocessor : public GeneralPostprocessor
{
public:

  PlenumPressurePostprocessor(const std::string & name, InputParameters parameters);

  virtual ~PlenumPressurePostprocessor(){}

  virtual void initialSetup() {}

  virtual void residualSetup() {}

  virtual void timestepSetup() {}

  virtual void execute();

  virtual void initialize();

  virtual PostprocessorValue getValue()
  {
    return _my_value;
  }

protected:

  void init();

  Real & _n0; // The initial number of moles of gas.

  const Real _initial_pressure;

  std::vector<Real*> _material_input;

  const Real _R;

  const Real & _temperature;

  const Real & _volume;

  Real _start_time;
  const Real _startup_time;

  ReportableValue * const _initial_moles;

  const unsigned _refab_needed;
  Real & _refab_gas_released;
  const std::vector<Real> _refab_time;
  const std::vector<Real> _refab_pressure;
  const std::vector<Real> _refab_temperature;
  const std::vector<Real> _refab_volume;
  const std::vector<unsigned> _refab_type;
  unsigned & _refab_counter;

  Real & _my_value;

  bool _initialized;

};

template<>
InputParameters validParams<PlenumPressurePostprocessor>();

#endif //PLENUMRESSUREPOSTPROCESSOR_H
