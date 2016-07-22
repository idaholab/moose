/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/


#ifndef POROUSFLOWSINK_H
#define POROUSFLOWSINK_H

#include "IntegratedBC.h"
#include "Function.h"
#include "PorousFlowDictator.h"

// Forward Declarations
class PorousFlowSink;

template<>
InputParameters validParams<PorousFlowSink>();

/**
 * Applies a flux sink to a boundary.
 * The strength of the flux is specified by flux_function.
 * In addition, this sink can be multiplied by:
 *  (1) the relative permeability of the fluid at the nodes
 *  (2) perm_nn*density/viscosity (the so-called mobility)
 *      where perm_nn is the permeability tensor projected
 *      to the normal direction.
 *  (3) the mass_fraction of a component at the nodes
 */
class PorousFlowSink : public IntegratedBC
{
public:

  PorousFlowSink(const InputParameters & parameters);

protected:
  virtual void computeResidual();

  virtual Real computeQpResidual();

  virtual void computeJacobian();

  virtual void computeJacobianBlock(unsigned int jvar);

  virtual Real computeQpJacobian();

  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

  /// PorousFlow UserObject
  const PorousFlowDictator & _dictator;

  /// The phase number
  const unsigned int _ph;

  /// Whether the flux will be multiplied by the mass fraction
  const bool _use_mass_fraction;

  /// The component number (only used if _use_mass_fraction==true)
  const unsigned int _sp;

  /// whether to multiply the sink flux by permeability*density/viscosity
  const bool _use_mobility;

  /// whether to multiply the sink flux by relative permeability
  const bool _use_relperm;

  /// The flux
  Function & _m_func;

  /// Permeability of porous material
  const MaterialProperty<RealTensorValue> & _permeability;

  /// d(Permeability)/d(PorousFlow variable)
  const MaterialProperty<std::vector<RealTensorValue> > & _dpermeability_dvar;

  /// Fluid density for each phase (at the node)
  const MaterialProperty<std::vector<Real> > & _fluid_density_node;

  /// d(Fluid density for each phase (at the node))/d(PorousFlow variable)
  const MaterialProperty<std::vector<std::vector<Real> > > & _dfluid_density_node_dvar;

  /// Viscosity of each component in each phase
  const MaterialProperty<std::vector<Real> > & _fluid_viscosity;

  /// d(Viscosity of each component in each phase)/d(PorousFlow variable)
  const MaterialProperty<std::vector<std::vector<Real> > > & _dfluid_viscosity_dvar;

  /// Relative permeability of each phase
  const MaterialProperty<std::vector<Real> > & _relative_permeability;

  /// d(Relative permeability of each phase)/d(PorousFlow variable)
  const MaterialProperty<std::vector<std::vector<Real> > > & _drelative_permeability_dvar;

  /// Mass fraction of each component in each phase
  const MaterialProperty<std::vector<std::vector<Real> > > & _mass_fractions;

  /// d(Mass fraction of each component in each phase)/d(PorousFlow variable)
  const MaterialProperty<std::vector<std::vector<std::vector<Real> > > > & _dmass_fractions_dvar;

  /// Node Number information held in the quadpoints of the Materials
  const MaterialProperty<unsigned int> & _node_number;

  /// _qp_map[node_number] = the quadpoint in the PorousFlow Materials that hold info for node = node_number
  std::vector<int> _qp_map;

  /// derivative of residual with respect to the jvar variable
  Real jac(unsigned int jvar);

  /// The flux gets multiplied by this quantity
  virtual Real multiplier();

  /// d(multiplier)/d(Porous flow variable pvar)
  virtual Real dmultiplier_dvar(unsigned int pvar);
};

#endif //POROUSFLOWSINK_H
