#ifndef RICHARDSMATERIAL_H
#define RICHARDSMATERIAL_H

#include "Material.h"

#include "RichardsDensity.h"
#include "RichardsRelPerm.h"
#include "RichardsSeff.h"
#include "RichardsSat.h"
#include "RichardsSUPG.h"

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
  std::vector<Real> _material_viscosity;

  unsigned int _num_p; 

  RealVectorValue _material_gravity;
  // Following is for SUPG
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
  MaterialProperty<std::vector<Real> > & _viscosity;
  MaterialProperty<RealVectorValue> & _gravity;

  MaterialProperty<std::vector<unsigned int> > & _p_var_nums;
  MaterialProperty<std::vector<int> > & _mat_var_num;

  MaterialProperty<std::vector<Real> > & _density_old;

  MaterialProperty<std::vector<Real> > & _density;
  MaterialProperty<std::vector<Real> > & _ddensity; // d(density)/dp
  MaterialProperty<std::vector<Real> > & _d2density; // d^2(density)/dp^2

  MaterialProperty<std::vector<Real> > & _seff_old; // old effective saturation

  MaterialProperty<std::vector<Real> > & _seff; // effective saturation
  MaterialProperty<std::vector<std::vector<Real> > > & _dseff; // d(seff)/dp
  MaterialProperty<std::vector<std::vector<std::vector<Real> > > > & _d2seff; // d^2(seff)/dp^2

  MaterialProperty<std::vector<Real> >& _sat_old; // old saturation

  MaterialProperty<std::vector<Real> >& _sat; // saturation
  MaterialProperty<std::vector<std::vector<Real> > >& _dsat; // d(saturation)/dp
  MaterialProperty<std::vector<std::vector<std::vector<Real> > > >& _d2sat; // d^2(saturation)/dp^2

  MaterialProperty<std::vector<Real> > & _rel_perm; // relative permeability
  MaterialProperty<std::vector<Real> > & _drel_perm; // d(relperm)/dSeff
  MaterialProperty<std::vector<Real> > & _d2rel_perm; // d^2(relperm)/dSeff^2

  MaterialProperty<std::vector<RealVectorValue> > & _tauvel_SUPG; // tauSUPG * velSUPG
  MaterialProperty<std::vector<RealTensorValue> > & _dtauvel_SUPG_dgradp; // d (_tauvel_SUPG)/d(_grad_p)
  MaterialProperty<std::vector<RealVectorValue> > & _dtauvel_SUPG_dp; // d (_tauvel_SUPG)/d(p)

  std::vector<unsigned int> _pressure_vars;
  std::vector<VariableValue *> _pressure_vals;
  std::vector<VariableValue *> _pressure_old_vals;

  std::vector<const RichardsRelPerm *> _material_relperm_UO;
  std::vector<const RichardsSeff *> _material_seff_UO;
  std::vector<const RichardsSat *> _material_sat_UO;
  std::vector<const RichardsDensity *> _material_density_UO;
  std::vector<const RichardsSUPG *> _material_SUPG_UO;

  std::vector<int> _material_var_num;

  std::vector<VariableGradient *> _grad_p;

  RealVectorValue velSUPG(VectorValue<double> gradp, Real dens, unsigned int qp);
  RealTensorValue velPrimeSUPG();
  Real tauSUPG(VectorValue<double> gradp, Real dens, Real SUPGp, unsigned int qp);
  RealVectorValue tauPrimeSUPG(VectorValue<double> gradp, Real dens, Real SUPGp, unsigned int qp);

};

#endif //RICHARDSMATERIAL_H
