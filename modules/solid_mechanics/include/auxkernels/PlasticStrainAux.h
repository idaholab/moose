#ifndef PLASTICSTRAINAUX_H
#define PLASTICSTRAINAUX_H

#include "AuxKernel.h"

class PlasticStrainAux;

template<>
InputParameters validParams<PlasticStrainAux>();


class PlasticStrainAux : public AuxKernel
{
public:
  PlasticStrainAux( const std::string & name, InputParameters parameters );

  virtual ~PlasticStrainAux() {}

protected:
  virtual Real computeValue();

  const unsigned _index;
  MaterialProperty<RealTensorValue> & _plastic_strain;

};

#endif // PLASTICSTRAINAUX_H
