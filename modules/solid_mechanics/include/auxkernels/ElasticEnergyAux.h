#ifndef ELASITCENERGYAUX_H
#define ELASTICENERGYAUX_H

#include "AuxKernel.h"

class ElasticEnergyAux;
class SymmTensor;

template<>
InputParameters validParams<ElasticEnergyAux>();


class ElasticEnergyAux : public AuxKernel
{
public:
  ElasticEnergyAux( const std::string & name, InputParameters parameters );

  virtual ~ElasticEnergyAux() {}

protected:
  virtual Real computeValue();

  MaterialProperty<SymmTensor> & _stress;
  VariableGradient & _grad_disp_x;
  VariableGradient & _grad_disp_y;
  VariableGradient & _grad_disp_z;

};

#endif // ELASTICENERGYAUX_H
