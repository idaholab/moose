/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/


#ifndef POROUSFLOWPOROSITYHM_H
#define POROUSFLOWPOROSITYHM_H

#include "DerivativeMaterialInterface.h"
#include "Material.h"

#include "PorousFlowDictator.h"
#include "RankTwoTensor.h"

//Forward Declarations
class PorousFlowPorosityHM;

template<>
InputParameters validParams<PorousFlowPorosityHM>();

/**
 * Material designed to provide the porosity
 * biot + (phi0 - biot)*exp(-vol_strain + (biot-1)effective_porepressure/solid_bulk)
 */
class PorousFlowPorosityHM : public DerivativeMaterialInterface<Material>
{
public:
  PorousFlowPorosityHM(const InputParameters & parameters);

protected:

  /// porosity at zero strain and zero porepressure
  const Real _phi0;

  /// The dictator UserObject for the Porous-Flow simulation
  const PorousFlowDictator & _dictator_UO;

  /// biot coefficient
  const Real _biot;

  /// drained bulk modulus of the porous skeleton
  const Real _solid_bulk;

  /// number of porous-flow variables
  const unsigned int _num_var;

  /// short-hand number (biot-1)/solid_bulk
  const Real _coeff;

  /// number of displacement variables
  unsigned int _ndisp;

  /// variable number of the displacements variables
  std::vector<unsigned int> _disp_var_num;

  /// strain
  const MaterialProperty<RankTwoTensor> & _strain;

  /// effective nodal porepressure
  const MaterialProperty<Real> & _pf_nodal;

  /// d(effective nodal porepressure)/(d porflow variable)
  const MaterialProperty<std::vector<Real> > & _dpf_nodal_dvar;

  /// effective qp porepressure
  const MaterialProperty<Real> & _pf_qp;

  /// d(effective qp porepressure)/(d porflow variable)
  const MaterialProperty<std::vector<Real> > & _dpf_qp_dvar;

  /// output value of nodal porosity
  MaterialProperty<Real> & _porosity_nodal;

  /// old value of nodal porosity
  MaterialProperty<Real> & _porosity_nodal_old;

  /// d(nodal porosity)/d(PorousFlow variable)
  MaterialProperty<std::vector<Real> > & _dporosity_nodal_dvar;

  /// d(nodal porosity)/d(grad(PorousFlow variable))
  MaterialProperty<std::vector<RealGradient> > & _dporosity_nodal_dgradvar;

  /// output value of qp porosity
  MaterialProperty<Real> & _porosity_qp;

  /// old value of qp porosity
  MaterialProperty<Real> & _porosity_qp_old;

  /// d(qp porosity)/d(PorousFlow variable)
  MaterialProperty<std::vector<Real> > & _dporosity_qp_dvar;

  /// d(qp porosity)/d(grad(PorousFlow variable))
  MaterialProperty<std::vector<RealGradient> > & _dporosity_qp_dgradvar;

  virtual void initQpStatefulProperties();
  virtual void computeQpProperties();
};

#endif //POROUSFLOWPOROSITYHM_H
