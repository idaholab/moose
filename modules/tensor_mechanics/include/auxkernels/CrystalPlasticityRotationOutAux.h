#ifndef CRYSTALPLASTICITYROTATIONOUTAUX_H
#define CRYSTALPLASTICITYROTATIONOUTAUX_H

#include "AuxKernel.h"
#include "FiniteStrainCrystalPlasticity.h"

class CrystalPlasticityRotationOutAux;

template<>
InputParameters validParams<CrystalPlasticityRotationOutAux>();

class CrystalPlasticityRotationOutAux : public AuxKernel
{
public:
  CrystalPlasticityRotationOutAux(const std::string & name, InputParameters parameters);
  virtual ~CrystalPlasticityRotationOutAux() {}

protected:
  virtual Real computeValue();

private:
  std::string _rotout_file_name;
  unsigned int _out_freq;
  MaterialProperty<RankTwoTensor> & _update_rot;
};

#endif //CRYSTALPLASTICITYROTATIONOUTAUX_H//
