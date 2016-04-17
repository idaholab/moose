/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/


#ifndef POROUSFLOWMATERIAL1PAHSEMD_GAUSSIAN_H
#define POROUSFLOWMATERIAL1PAHSEMD_GAUSSIAN_H

#include "DerivativeMaterialInterface.h"
#include "Material.h"

#include "PorousFlowDictator.h"

//Forward Declarations
class PorousFlowMaterial1PhaseMD_Gaussian;

template<>
InputParameters validParams<PorousFlowMaterial1PhaseMD_Gaussian>();

/**
 * Material designed to calculate fluid-phase porepressure and saturation
 * for the single-phase situation, assuming a Gaussian capillary suction
 * function and assuming the independent variable is log(mass density) and
 * assuming the fluid has a constant bulk modulus
 */
class PorousFlowMaterial1PhaseMD_Gaussian : public DerivativeMaterialInterface<Material>
{
public:
  PorousFlowMaterial1PhaseMD_Gaussian(const InputParameters & parameters);

protected:

  /// number of phases (=1 for this class)
  const unsigned int _num_ph;

  /// Gaussian parameter: saturation = exp(-(al*p)^2)
  const Real _al;

  /// _al2 = al*al
  const Real _al2;

  /// fluid density = _dens0*exp(P/_bulk)
  const Real _logdens0;

  /// fluid density = _dens0*exp(P/_bulk)
  const Real _bulk;

  /// 1/_bulk/_al
  const Real _recip_bulk;

  /// (1/_bulk)^2
  const Real _recip_bulk2;

  /// Nodal value of mass-density of the fluid phase
  const VariableValue & _md_var;

  /// Quadpoint value of mass-density of the fluid phase
  const VariableValue & _qp_md_var;

  /// Gradient(_mass-density at quadpoints)
  const VariableGradient & _gradmd_var;

  /// Moose variable number of the mass-density
  const unsigned int _md_varnum;

  /// The variable names UserObject for the Porous-Flow variables
  const PorousFlowDictator & _dictator_UO;

  /// nodal porepressure of the phases
  MaterialProperty<std::vector<Real> > & _porepressure;

  /// old value of nodal porepressure of the phases
  MaterialProperty<std::vector<Real> > & _porepressure_old;

  /// quadpoint porepressure of the phases
  MaterialProperty<std::vector<Real> > & _porepressure_qp;

  /// grad(p) at the quadpoints
  MaterialProperty<std::vector<RealGradient> > & _gradp;

  /// d(nodal porepressure)/d(nodal porflow variable)
  MaterialProperty<std::vector<std::vector<Real> > > & _dporepressure_dvar;

  /// d(quadpoint porepressure)/d(quadpoint porflow variable)
  MaterialProperty<std::vector<std::vector<Real> > > & _dporepressure_qp_dvar;

  /// d(grad porepressure)/d(grad porflow variable)
  MaterialProperty<std::vector<std::vector<Real> > > & _dgradp_dgradv;

  /// d(grad porepressure)/d(porflow variable)
  MaterialProperty<std::vector<std::vector<RealGradient> > > & _dgradp_dv;

  /// nodal saturation of the phases
  MaterialProperty<std::vector<Real> > & _saturation;

  /// old value of nodal saturation of the phases
  MaterialProperty<std::vector<Real> > & _saturation_old;

  /// quadpoint saturation of the phases
  MaterialProperty<std::vector<Real> > & _saturation_qp;

  /// grad(s) at the quadpoints
  MaterialProperty<std::vector<RealGradient> > & _grads;

  /// d(nodal saturation)/d(nodal porflow variable)
  MaterialProperty<std::vector<std::vector<Real> > > & _dsaturation_dvar;

  /// d(quadpoint saturation)/d(quadpoint porflow variable)
  MaterialProperty<std::vector<std::vector<Real> > > & _dsaturation_qp_dvar;

  /// d(grad saturation)/d(grad porflow variable)
  MaterialProperty<std::vector<std::vector<Real> > > & _dgrads_dgradv;

  /// d(grad saturation)/d(porflow variable)
  MaterialProperty<std::vector<std::vector<RealGradient> > > & _dgrads_dv;


  virtual void initQpStatefulProperties();
  virtual void computeQpProperties();

 private:
  void buildPS();
};

#endif //POROUSFLOWMATERIAL1PAHSEMD_GAUSSIAN_H
