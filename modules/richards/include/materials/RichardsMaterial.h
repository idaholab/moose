#ifndef RICHARDSMATERIAL_H
#define RICHARDSMATERIAL_H

#include "Material.h"

#include "RichardsDensity.h"
#include "RichardsRelPerm.h"
#include "RichardsSeff.h"
#include "RichardsSat.h"

//Forward Declarations
class RichardsMaterial;

template<>
InputParameters validParams<RichardsMaterial>();

class RichardsMaterial : public Material
{
public:
  RichardsMaterial(const std::string & name,
                  InputParameters parameters);

protected:
  //virtual void computeQpProperties();
  virtual void computeProperties();


private:

  Real _material_por;
  RealTensorValue _material_perm;
  const RichardsRelPerm & _material_relperm_UO;
  const RichardsSeff & _material_seff_UO;
  const RichardsSat & _material_sat_UO;
  const RichardsDensity & _material_density_UO;
  Real _material_dens0;
  Real _material_viscosity;
  RealVectorValue _material_gravity;

  VariableValue& _pressure;

  VariableValue& _pressure_old;

  // Following is for SUPG
  bool _doing_SUPG;
  Real _SUPG_pressure;
  VariableGradient& _grad_p;
  Real _trace_perm;

  // Reference to a pointer to an FEBase object from the _subproblem object.  Initialized in ctor.
  FEBase*& _fe;
  // Constant references to finite element mapping data
  const std::vector<Real>& _dxidx;
  const std::vector<Real>& _dxidy;
  const std::vector<Real>& _dxidz;
  const std::vector<Real>& _detadx;
  const std::vector<Real>& _detady;
  const std::vector<Real>& _detadz;
  const std::vector<Real>& _dzetadx; // Empty in 2D
  const std::vector<Real>& _dzetady; // Empty in 2D
  const std::vector<Real>& _dzetadz; // Empty in 2D

  MaterialProperty<Real> & _porosity;
  MaterialProperty<RealTensorValue> & _permeability;
  MaterialProperty<Real> & _dens0;
  MaterialProperty<Real> & _viscosity;
  MaterialProperty<RealVectorValue> & _gravity;

  MaterialProperty<Real> & _density_old;

  MaterialProperty<Real> & _density;
  MaterialProperty<Real> & _ddensity; // d(density)/dp
  MaterialProperty<Real> & _d2density; // d^2(density)/dp^2

  MaterialProperty<Real> & _seff_old; // old effective saturation

  MaterialProperty<Real> & _seff; // effective saturation
  MaterialProperty<Real> & _dseff; // d(seff)/dp
  MaterialProperty<Real> & _d2seff; // d^2(seff)/dp^2

  MaterialProperty<Real> & _sat_old; // old saturation

  MaterialProperty<Real> & _sat; // saturation
  MaterialProperty<Real> & _dsat; // d(saturation)/dp
  MaterialProperty<Real> & _d2sat; // d^2(saturation)/dp^2

  MaterialProperty<Real> & _rel_perm; // relative permeability
  MaterialProperty<Real> & _drel_perm; // d(relperm)/dSeff
  MaterialProperty<Real> & _d2rel_perm; // d^2(relperm)/dSeff^2

  MaterialProperty<RealVectorValue> & _vel_SUPG; // vector that points in direction of information propagation
  MaterialProperty<RealTensorValue> & _vel_prime_SUPG; // d (_vel_SUPG)/d(_grad_u)
  MaterialProperty<Real> & _tau_SUPG; // the "tau" SUPG parameter
  MaterialProperty<RealVectorValue> & _tau_prime_SUPG; // d (_tau_SUPG)/d(_grad_u)

  RealVectorValue velSUPG(unsigned qp);
  RealTensorValue velPrimeSUPG(unsigned qp);
  Real tauSUPG(unsigned qp);
  RealVectorValue tauPrimeSUPG(unsigned qp);

};

#endif //RICHARDSMATERIAL_H
