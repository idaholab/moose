// Original class author: A.M. Jokisaari,  O. Heinonen

#ifndef FINITESTRAINCRYSTALPLASTICITY_H
#define FINITESTRAINCRYSTALPLASTICITY_H

#include "FiniteStrainMaterial.h"

class FiniteStrainCrystalPlasticity;

template<>
InputParameters validParams<FiniteStrainCrystalPlasticity>();

class FiniteStrainCrystalPlasticity : public FiniteStrainMaterial
{
public:
  FiniteStrainCrystalPlasticity(const std::string & name, InputParameters parameters);

protected:
  virtual void computeQpStress();
  virtual void computeQpElasticityTensor();
  virtual void initQpStatefulProperties();
  virtual void calc_resid_jacob(const RankTwoTensor &, RankTwoTensor &, const RankTwoTensor &, RankTwoTensor &,
                                std::vector<Real> &, std::vector<Real> &, RankTwoTensor &, RankFourTensor &);
  virtual void getSlipIncrements(const std::vector<Real> &, std::vector<Real> &, std::vector<Real> &);
  virtual void updateGss(std::vector<Real> &);

  virtual void getSlipSystems();
  virtual void getEulerAngles();

  void getEulerRotations();
  RankFourTensor outerProduct(const RankTwoTensor & a, const RankTwoTensor & b);
  RankTwoTensor getMatRot(const RankTwoTensor & a);

  const unsigned int _nss;

  std::vector<Real> _gprops;
  std::vector<Real> _hprops;
  std::vector<Real> _flowprops;

  std::string _slip_sys_file_name;
  std::string _euler_angle_file_name;

  std::vector<Real> _mo;
  std::vector<Real> _no;

  std::vector<Real> _a0;
  std::vector<Real> _xm;

  Real _h0;
  Real _tau_sat;
  Real _tau_init;
  Real _r;
  Real _rtol;
  Real _gtol;
  Real _slip_incr_tol;

  MaterialProperty<RankTwoTensor> & _fp;
  MaterialProperty<RankTwoTensor> & _fp_old;
  MaterialProperty<RankTwoTensor> & _pk2;
  MaterialProperty<RankTwoTensor> & _pk2_old;
  MaterialProperty<RankTwoTensor> & _lag_e;
  MaterialProperty<std::vector<Real> > & _gss;
  MaterialProperty<std::vector<Real> > & _gss_old;
  MaterialProperty<Real> & _acc_slip;
  MaterialProperty<Real> & _acc_slip_old;
  MaterialProperty<RankTwoTensor> & _update_rot;
  MaterialProperty<RankTwoTensor> & _crysrot;
  MaterialProperty<RankTwoTensor> & _crysrot_old;
};

#endif //FINITESTRAINCRYSTALPLASTICITY_H
