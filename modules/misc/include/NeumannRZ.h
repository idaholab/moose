#ifndef NEUMANNRZ_H
#define NEUMANNRZ_H

#include "NeumannBC.h"


//Forward Declarations
class NeumannRZ;

template<>
InputParameters validParams<NeumannRZ>();

class NeumannRZ : public NeumannBC
{
public:

  NeumannRZ(const std::string & name,InputParameters parameters);

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  virtual Real computeQpOffDiagJacobian( unsigned jvar );

};
#endif //NEUMANNRZ_H
