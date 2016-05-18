/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef POROUSFLOWEFFECTIVEFLUIDPRESSURE_H
#define POROUSFLOWEFFECTIVEFLUIDPRESSURE_H

#include "DerivativeMaterialInterface.h"
#include "Material.h"

#include "PorousFlowDictator.h"

//Forward Declarations
class PorousFlowEffectiveFluidPressure;

template<>
InputParameters validParams<PorousFlowEffectiveFluidPressure>();

/**
 * Material designed to calculate the effective fluid pressure
 * that can be used in the mechanical effective-stress calculations
 * and other similar places.  This class computes
 * effective fluid pressure = sum_{phases}Saturation_{phase}*Porepressure_{phase}
 */
class PorousFlowEffectiveFluidPressure : public DerivativeMaterialInterface<Material>
{
public:
  PorousFlowEffectiveFluidPressure(const InputParameters & parameters);

protected:
  /// The dictator UserObject for the Porous-Flow variables
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

  /// nodal porepressure of each phase
  const MaterialProperty<std::vector<Real> > & _porepressure_nodal;

  /// d(nodal porepressure)/d(PorousFlow variable)
  const MaterialProperty<std::vector<std::vector<Real> > > & _dporepressure_nodal_dvar;

  /// nodal saturation of each phase
  const MaterialProperty<std::vector<Real> > & _saturation_nodal;

  /// d(nodal saturation)/d(PorousFlow variable)
  const MaterialProperty<std::vector<std::vector<Real> > > & _dsaturation_nodal_dvar;

  /// computed effective fluid pressure (at quadpoints)
  MaterialProperty<Real> & _pf_qp;

  /// d(_pf_qp)/d(PorousFlow variable)
  MaterialProperty<std::vector<Real> > & _dpf_qp_dvar;

  /// computed effective fluid pressure (at nodes)
  MaterialProperty<Real> & _pf_nodal;

  /// d(_pf_nodal)/d(PorousFlow variable)
  MaterialProperty<std::vector<Real> > & _dpf_nodal_dvar;

  virtual void computeQpProperties();
};

#endif //POROUSFLOWEFFECTIVEFLUIDPRESSURE_H
