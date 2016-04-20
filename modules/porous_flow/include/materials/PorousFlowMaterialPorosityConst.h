/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/


#ifndef PORFLOWMATERIALPOROSITYCONST_H
#define PORFLOWMATERIALPOROSITYCONST_H

#include "DerivativeMaterialInterface.h"
#include "Material.h"

#include "PorousFlowDictator.h"

//Forward Declarations
class PorousFlowMaterialPorosityConst;

template<>
InputParameters validParams<PorousFlowMaterialPorosityConst>();

/**
 * Material designed to provide the porosity
 * which is assumed constant
 */
class PorousFlowMaterialPorosityConst : public DerivativeMaterialInterface<Material>
{
public:
  PorousFlowMaterialPorosityConst(const InputParameters & parameters);

protected:

  /// constant value of porosity
  const Real _input_porosity;

  /// The variable names UserObject for the Porous-Flow variables
  const PorousFlowDictator & _porflow_name_UO;

  /// nodal porosity
  MaterialProperty<Real> & _porosity_nodal;

  /// old value of nodal porosity (which is, of course = _porosity in this case)
  MaterialProperty<Real> & _porosity_nodal_old;

  /// d(nodal porosity)/d(PorousFlow variable)
  MaterialProperty<std::vector<Real> > & _dporosity_nodal_dvar;

  /// d(nodal porosity)/d(PorousFlow variable)
  MaterialProperty<std::vector<RealGradient> > & _dporosity_nodal_dgradvar;

  /// qp porosity
  MaterialProperty<Real> & _porosity_qp;

  /// old value of qp porosity (which is, of course = _porosity in this case)
  MaterialProperty<Real> & _porosity_qp_old;

  /// d(qp porosity)/d(PorousFlow variable)
  MaterialProperty<std::vector<Real> > & _dporosity_qp_dvar;

  /// d(qp porosity)/d(PorousFlow variable)
  MaterialProperty<std::vector<RealGradient> > & _dporosity_qp_dgradvar;

  virtual void initQpStatefulProperties();
  virtual void computeQpProperties();
};

#endif //PORFLOWMATERIALPOROSITYCONST_H
