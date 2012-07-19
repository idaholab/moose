#ifndef RANKFOURAUX_H
#define RANKFOURAUX_H

#include "AuxKernel.h"
#include "RankFourTensor.h"

class RankFourAux;

/**
 * RankFourAux is designed to take the data in the RankFourTensor material
 * property, for example stiffness, and output the value for the
 * supplied indices.
 */

template<>
InputParameters validParams<RankFourAux>();

class RankFourAux : public AuxKernel
{
public:
  RankFourAux(const std::string & name, InputParameters parameters);

  virtual ~ RankFourAux() {}
  
protected:
  virtual Real computeValue();

private:  MaterialProperty<RankFourTensor> & _tensor;
  const int _i;
  const int _j;
  const int _k;
  const int _l;
};

#endif //RANKFOURAUX_H
