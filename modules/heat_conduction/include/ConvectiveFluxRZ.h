#ifndef CONVECTIVEFLUXRZ_H
#define CONVECTIVEFLUXRZ_H

#include "ConvectiveFluxBC.h"

//Forward Declarations
class ConvectiveFluxRZ;

template<>
InputParameters validParams<ConvectiveFluxRZ>();

class ConvectiveFluxRZ : public ConvectiveFluxBC
{
public:

  ConvectiveFluxRZ(const std::string & name, InputParameters parameters);

  virtual ~ConvectiveFluxRZ() {}

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  virtual Real computeQpOffDiagJacobian( unsigned jvar );

};

#endif //CONVECTIVEFLUXBC_H
