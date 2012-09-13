#ifndef RANKFOURTONKSAUX_H
#define RANKFOURTONKSAUX_H

#include "AuxKernel.h"
#include "ElasticityTensorR4.h"

class RankFourTonksAux;

/**
 * RankFourTonksAux is designed to take the data in the ElasticityTensorR4 material
 * property, for example stiffness, and output the value for the
 * supplied indices.
 */

template<>
InputParameters validParams<RankFourTonksAux>();

class RankFourTonksAux : public AuxKernel
{
public:
  RankFourTonksAux(const std::string & name, InputParameters parameters);

  virtual ~ RankFourTonksAux() {}
  
protected:
  virtual Real computeValue();

private:  MaterialProperty<ElasticityTensorR4> & _tensor;
  const int _i;
  const int _j;
  const int _k;
  const int _l;
};

#endif //RANKFOURTONKSAUX_H
