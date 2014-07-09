#ifndef CRYSTALPLASTICITYSLIPSYSAUX_H
#define CRYSTALPLASTICITYSLIPSYSAUX_H

#include "AuxKernel.h"
#include "FiniteStrainCrystalPlasticity.h"

class CrystalPlasticitySlipSysAux;

template<>
InputParameters validParams<CrystalPlasticitySlipSysAux>();

class CrystalPlasticitySlipSysAux : public AuxKernel
{
 public:
  CrystalPlasticitySlipSysAux(const std::string & name, InputParameters parameters);
  virtual ~CrystalPlasticitySlipSysAux() {}

 protected:
  virtual Real computeValue();

 private:
  MaterialProperty< std::vector<Real> > & _slipsysvar;
  const unsigned int _i;
};

#endif //CRYSTALPLASTICITYSLIPSYSAUX_H
