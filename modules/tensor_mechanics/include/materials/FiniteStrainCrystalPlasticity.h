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
  virtual void calc_resid_jacob(RankTwoTensor &, RankFourTensor &);
  virtual void getSlipIncrements();

  //Override to modify slip system resistance evolution
  virtual void update_slip_system_resistance();

  //Old function: Kept to avoid code break in computeQpStress
  virtual void updateGss();

  virtual void getSlipSystems();
  virtual void getEulerAngles();

  virtual void readFileInitSlipSysRes();
  virtual void getInitSlipSysRes();

  virtual void readFileFlowRateParams();
  virtual void getFlowRateParams();

  virtual void readFileHardnessParams();
  virtual void getHardnessParams();

  virtual void initSlipSysProps();
  virtual void initAdditionalProps();

  virtual void preSolveQp();
  virtual void solveQp();
  virtual void postSolveQp();

  virtual void preSolveStatevar();
  virtual void solveStatevar();
  virtual void postSolveStatevar();

  virtual void preSolveStress();
  virtual void solveStress();
  virtual void postSolveStress();

  virtual void calcResidual( RankTwoTensor & );
  virtual void calcJacobian( RankFourTensor & );

  void getEulerRotations();
  RankFourTensor outerProduct(const RankTwoTensor & a, const RankTwoTensor & b);


  RankTwoTensor get_current_rotation(const RankTwoTensor & a);

  ////Old function: Kept to avoid code break in computeQpStress
  RankTwoTensor getMatRot(const RankTwoTensor & a);




  const unsigned int _nss;

  std::vector<Real> _gprops;
  std::vector<Real> _hprops;
  std::vector<Real> _flowprops;

  std::string _slip_sys_file_name;//File should contain slip plane normal and direction. See test.

  std::string _slip_sys_res_prop_file_name;
  //File should contain initial values of the slip system resistances.
  std::string _slip_sys_flow_prop_file_name;
  /*File should contain values of the flow rate equation parameters.
    Values for every slip system must be provided.
    Should have the same order of slip systens as in slip_sys_file. See test.
    The option of reading all the properties from .i is still present.
   */
  std::string _slip_sys_hard_prop_file_name;
  //    The hardening parameters in this class are read from .i file. The user can override to read from file.

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

  unsigned int _maxiter;
  unsigned int _maxiterg;

  unsigned int _num_slip_sys_flowrate_props;

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


  RankTwoTensor _fe, _fp_old_inv, _fp_inv;
  std::vector< Real > _slip_incr, _tau, _dslipdtau;
  //  std::vector< std::vector < std::vector <Real> > > _s0;
  std::vector<RankTwoTensor> _s0;

};

#endif //FINITESTRAINCRYSTALPLASTICITY_H
