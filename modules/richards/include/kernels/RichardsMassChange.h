#ifndef RICHARDSMASSCHANGE
#define RICHARDSMASSCHANGE

#include "Kernel.h"

// Forward Declarations
class RichardsMassChange;

template<>
InputParameters validParams<RichardsMassChange>();

class RichardsMassChange : public Kernel
{
public:

  RichardsMassChange(const std::string & name,
                        InputParameters parameters);

  //virtual void computeJacobian();

protected:
  virtual Real computeQpResidual();

  virtual Real computeQpJacobian();

  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

  unsigned int _this_var_num;

  bool _lumping;
  bool _use_supg;

  MaterialProperty<Real> &_porosity;

  MaterialProperty<std::vector<unsigned int> > &_p_var_nums;

  MaterialProperty<std::vector<Real> > &_sat_old;

  MaterialProperty<std::vector<Real> > &_sat;
  MaterialProperty<std::vector<std::vector<Real> > > &_dsat;
  MaterialProperty<std::vector<std::vector<std::vector<Real> > > > &_d2sat;

  MaterialProperty<std::vector<Real> > &_density_old;

  MaterialProperty<std::vector<Real> > &_density;
  MaterialProperty<std::vector<Real> > &_ddensity;
  MaterialProperty<std::vector<Real> > &_d2density;

  MaterialProperty<std::vector<RealVectorValue> >&_tauvel_SUPG;
  MaterialProperty<std::vector<RealTensorValue> >&_dtauvel_SUPG_dgradp;
  MaterialProperty<std::vector<RealVectorValue> >&_dtauvel_SUPG_dp;

};

#endif //RICHARDSMASSCHANGE
