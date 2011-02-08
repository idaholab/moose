#ifndef PRESSURERZ_H
#define PRESSURERZ_H

#include "Pressure.h"

//Forward Declarations
class PressureRZ;

template<>
InputParameters validParams<PressureRZ>();

class PressureRZ : public Pressure
{
public:

  PressureRZ(const std::string & name, InputParameters parameters);

  virtual ~PressureRZ(){}

protected:

  virtual Real computeQpResidual();

};

#endif //PRESSURE_H
