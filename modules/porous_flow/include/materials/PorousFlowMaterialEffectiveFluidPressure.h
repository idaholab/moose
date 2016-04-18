/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/


#ifndef PORFLOWMATERIALEFFECTIVEFLUIDPRESSURE_H
#define PORFLOWMATERIALEFFECTIVEFLUIDPRESSURE_H

#include "DerivativeMaterialInterface.h"
#include "Material.h"

#include "PorousFlowDictator.h"

//Forward Declarations
class PorousFlowMaterialEffectiveFluidPressure;

template<>
InputParameters validParams<PorousFlowMaterialEffectiveFluidPressure>();

/**
 * Material designed to calculate fluid density
 * from porepressure, assuming constant bulk modulus
 * for the fluid.
 */
class PorousFlowMaterialEffectiveFluidPressure : public DerivativeMaterialInterface<Material>
{
public:
  PorousFlowMaterialEffectiveFluidPressure(const InputParameters & parameters);

protected:

  /// The variable names UserObject for the Porous-Flow variables
  const PorousFlowDictator & _dictator_UO;

  /// number of phases in this simulation
  const unsigned int _num_ph;

  /// number of porous-flow variables in this simulation
  const unsigned int _num_var;

  /// quadpoint porepressure of each phase
  const MaterialProperty<std::vector<Real> > & _porepressure_qp;

  /// d(quadpoint porepressure)/d(PorousFlow variable)
  const MaterialProperty<std::vector<std::vector<Real> > > & _dporepressure_qp_dvar;

  /// quadpoint saturation of each phase
  const MaterialProperty<std::vector<Real> > & _saturation_qp;

  /// d(quadpoint saturation)/d(PorousFlow variable)
  const MaterialProperty<std::vector<std::vector<Real> > > & _dsaturation_qp_dvar;

  /// computed effective fluid pressure (at quadpoints)
  MaterialProperty<Real> & _pf;

  /// d(_pf)/d(PorousFlow variable)
  MaterialProperty<std::vector<Real> > & _dpf_dvar;

  virtual void computeQpProperties();
};

#endif //PORFLOWMATERIALEFFECTIVEFLUIDPRESSURE_H
