#ifndef GBEVOLUTION_H
#define GBEVOLUTION_H

#include "Material.h"

//Forward Declarations
class GBEvolution;

template<>
InputParameters validParams<GBEvolution>();

class GBEvolution : public Material
{
public:

  GBEvolution(const std::string & name,
          InputParameters parameters);

protected:
  virtual void computeProperties();

private:
  //VariableValue & _cg;

  Real _temp;
  Real _f0s;
  Real _wGB;
  Real _length_scale;
  Real _time_scale;
  Real _GBmob0;
  Real _Q;
  Real _GBenergy;
  bool _has_T;
  Real _GBMobility;
  Real _molar_vol;

  VariableValue * _T; //pointer rather than reference

  MaterialProperty<Real> & _sigma;
  MaterialProperty<Real> & _M_GB;
  MaterialProperty<Real> & _kappa;
  MaterialProperty<Real> & _gamma;
  MaterialProperty<Real> & _L;
  MaterialProperty<Real> & _l_GB;
  MaterialProperty<Real> & _mu;
  MaterialProperty<Real> & _entropy_diff;
  MaterialProperty<Real> & _molar_volume;
  MaterialProperty<Real> & _act_wGB;
  MaterialProperty<Real> & _tgrad_corr_mult;

  Real _kb;

};

#endif //GBEVOLUTION_H
