/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef POROUSFLOWPOROSITYBASE_H
#define POROUSFLOWPOROSITYBASE_H

#include "PorousFlowMaterialVectorBase.h"

//Forward Declarations
class PorousFlowPorosityBase;

template<>
InputParameters validParams<PorousFlowPorosityBase>();

/**
 * Base class Material designed to provide the porosity.
 */
class PorousFlowPorosityBase : public PorousFlowMaterialVectorBase
{
public:
  PorousFlowPorosityBase(const InputParameters & parameters);

protected:
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
};

#endif //POROUSFLOWPOROSITYBASE_H
