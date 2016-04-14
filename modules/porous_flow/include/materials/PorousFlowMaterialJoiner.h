/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/


#ifndef PORFLOWMATERIALJOINER_H
#define PORFLOWMATERIALJOINER_H

#include "DerivativeMaterialInterface.h"
#include "Material.h"

#include "PorousFlowDictator.h"

//Forward Declarations
class PorousFlowMaterialJoiner;

template<>
InputParameters validParams<PorousFlowMaterialJoiner>();

/**
 * Material designed to form a std::vector of property
 * from the individual phase densities
 */
class PorousFlowMaterialJoiner : public DerivativeMaterialInterface<Material>
{
public:
  PorousFlowMaterialJoiner(const InputParameters & parameters);

protected:

  /// The variable names UserObject for the Porous-Flow variables
  const PorousFlowDictator & _porflow_name_UO;

  const unsigned int _num_phases;

  const std::string _pf_prop;

  /// computed property of the phase
  MaterialProperty<std::vector<Real> > & _property;

  /// d(property)/d(PorousFlow variable)
  MaterialProperty<std::vector<std::vector<Real> > > & _dproperty_dvar;

  /// property of each phase
  std::vector<const MaterialProperty<Real> *> _phase_property;

  /// d(property of each phase)/d(var)
  std::vector<const MaterialProperty<std::vector<Real> > *> _dphase_property_dvar;

  virtual void initQpStatefulProperties();
  virtual void computeQpProperties();
};

#endif //PORFLOWMATERIALJOINER_H
