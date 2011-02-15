#ifndef PLENUMPRESSURERZ_H
#define PLENUMPRESSURERZ_H

#include "PlenumPressure.h"

//Forward Declarations
class PlenumPressureRZ;

template<>
InputParameters validParams<PlenumPressureRZ>();

class PlenumPressureRZ : public PlenumPressure
{
public:

  PlenumPressureRZ(const std::string & name, InputParameters parameters);

  virtual ~PlenumPressureRZ(){}

protected:

  virtual Real computeQpResidual();

};

#endif //PLENUMPRESSURE_H
