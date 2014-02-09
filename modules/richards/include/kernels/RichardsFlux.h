/*****************************************/
/* Written by andrew.wilkins@csiro.au    */
/* Please contact me if you make changes */
/*****************************************/

#ifndef RICHARDSFLUX
#define RICHARDSFLUX

#include "Kernel.h"
#include "RichardsPorepressureNames.h"

// Forward Declarations
class RichardsFlux;

template<>
InputParameters validParams<RichardsFlux>();

class RichardsFlux : public Kernel
{
public:

  RichardsFlux(const std::string & name,
                        InputParameters parameters);


protected:
  virtual Real computeQpResidual();

  virtual Real computeQpJacobian();

  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

  const RichardsPorepressureNames & _pp_name_UO;
  unsigned int _pvar;

  MaterialProperty<std::vector<Real> > &_viscosity;
  MaterialProperty<RealVectorValue> &_gravity;
  MaterialProperty<RealTensorValue> & _permeability;

  MaterialProperty<std::vector<Real> > &_seff;
  MaterialProperty<std::vector<std::vector<Real> > > &_dseff;
  MaterialProperty<std::vector<std::vector<std::vector<Real> > > > &_d2seff;

  MaterialProperty<std::vector<Real> > &_rel_perm;
  MaterialProperty<std::vector<Real> > &_drel_perm;
  MaterialProperty<std::vector<Real> > &_d2rel_perm;

  MaterialProperty<std::vector<Real> > &_density;
  MaterialProperty<std::vector<Real> > &_ddensity;
  MaterialProperty<std::vector<Real> > &_d2density;

  VariableSecond & _second_u;
  VariablePhiSecond & _second_phi;

  MaterialProperty<std::vector<RealVectorValue> >&_tauvel_SUPG;
  MaterialProperty<std::vector<RealTensorValue> >&_dtauvel_SUPG_dgradp;
  MaterialProperty<std::vector<RealVectorValue> >&_dtauvel_SUPG_dp;

 private:
  Real mobility(Real density, Real relperm);
  Real dmobility_dp(Real density, Real ddensity_dp, Real relperm, Real drelperm_dp);
  Real d2mobility_dp2(Real density, Real ddensity_dp, Real d2density_dp2, Real relperm, Real drelperm_dp, Real d2relperm_dp2);
};

#endif //RICHARDSFLUX
