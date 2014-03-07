/*****************************************/
/* Written by andrew.wilkins@csiro.au    */
/* Please contact me if you make changes */
/*****************************************/

#ifndef RICHARDSMATERIAL_H
#define RICHARDSMATERIAL_H

#include "Material.h"

#include "RichardsPorepressureNames.h"
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

  virtual void computeProperties();


private:

  Real _material_por;
  VariableValue * _por_change;
  VariableValue * _por_change_old;

  RealTensorValue _material_perm;
  std::vector<Real> _material_viscosity;

  const RichardsPorepressureNames & _pp_name_UO;
  unsigned int _num_p;

  RealVectorValue _material_gravity;
  // Following is for SUPG
  Real _trace_perm;

  MaterialProperty<Real> & _porosity;
  MaterialProperty<Real> & _porosity_old;
  MaterialProperty<RealTensorValue> & _permeability;
  MaterialProperty<std::vector<Real> > & _viscosity;
  MaterialProperty<RealVectorValue> & _gravity;

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

  std::vector<VariableValue *> _perm_change;

  std::vector<VariableValue *> _pressure_vals;
  std::vector<VariableValue *> _pressure_old_vals;
  std::vector<VariableGradient *> _grad_p;

  std::vector<const RichardsRelPerm *> _material_relperm_UO;
  std::vector<const RichardsSeff *> _material_seff_UO;
  std::vector<const RichardsSat *> _material_sat_UO;
  std::vector<const RichardsDensity *> _material_density_UO;
  std::vector<const RichardsSUPG *> _material_SUPG_UO;



};

#endif //RICHARDSMATERIAL_H
