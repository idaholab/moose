/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef POROUSFLOWPOROSITYUNITY_H
#define POROUSFLOWPOROSITYUNITY_H

#include "DerivativeMaterialInterface.h"
#include "Material.h"

#include "PorousFlowDictator.h"

//Forward Declarations
class PorousFlowPorosityUnity;

template<>
InputParameters validParams<PorousFlowPorosityUnity>();

/**
 * Base class Material designed to provide the porosity.
 * In this class porosity = 1
 */
class PorousFlowPorosityUnity : public DerivativeMaterialInterface<Material>
{
public:
  PorousFlowPorosityUnity(const InputParameters & parameters);

protected:
  /// The variable names UserObject for the Porous-Flow variables
  const PorousFlowDictator & _dictator_UO;

  /// nodal porosity
  MaterialProperty<Real> & _porosity_nodal;

  /// old value of nodal porosity (which is, of course = _porosity in this case)
  MaterialProperty<Real> & _porosity_nodal_old;

  /// d(nodal porosity)/d(PorousFlow variable)
  MaterialProperty<std::vector<Real> > & _dporosity_nodal_dvar;

  /// d(nodal porosity)/d(PorousFlow variable)
  MaterialProperty<std::vector<RealGradient> > & _dporosity_nodal_dgradvar;

  /// qaudpoint porosity
  MaterialProperty<Real> & _porosity_qp;

  /// old value of quadpoint porosity (which is, of course = _porosity in this case)
  MaterialProperty<Real> & _porosity_qp_old;

  /// d(quadpoint porosity)/d(PorousFlow variable)
  MaterialProperty<std::vector<Real> > & _dporosity_qp_dvar;

  /// d(quadpoint porosity)/d(PorousFlow variable)
  MaterialProperty<std::vector<RealGradient> > & _dporosity_qp_dgradvar;

  virtual void initQpStatefulProperties();
  virtual void computeQpProperties();
};

#endif //POROUSFLOWPOROSITYUNITY_H
