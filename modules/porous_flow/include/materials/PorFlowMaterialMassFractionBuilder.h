/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/


#ifndef PORFLOWMATERIALMASSFRACTIONBUILDER_H
#define PORFLOWMATERIALMASSFRACTIONBUILDER_H

#include "DerivativeMaterialInterface.h"
#include "Material.h"

#include "PorFlowVarNames.h"

//Forward Declarations
class PorFlowMaterialMassFractionBuilder;

template<>
InputParameters validParams<PorFlowMaterialMassFractionBuilder>();

/**
 * Material designed to form a std::vector<std::vector>
 * of mass fractions from the individual mass fraction variables
 */
class PorFlowMaterialMassFractionBuilder : public DerivativeMaterialInterface<Material>
{
public:
  PorFlowMaterialMassFractionBuilder(const InputParameters & parameters);

protected:

  unsigned int _num_phases;

  unsigned int _num_components;

  /// The variable names UserObject for the Porous-Flow variables
  const PorFlowVarNames & _porflow_name_UO;

  MaterialProperty<std::vector<std::vector<Real> > > & _mass_frac;
  MaterialProperty<std::vector<std::vector<Real> > > & _mass_frac_old;
  MaterialProperty<std::vector<std::vector<std::vector<Real> > > > & _dmass_frac_dvar;

  unsigned int _num_passed_mf_vars;

  std::vector<unsigned int> _mf_vars_num;
  std::vector<const VariableValue *> _mf_vars;

  virtual void initQpStatefulProperties();
  virtual void computeQpProperties();

 private:
  void build_mass_frac(unsigned int qp);
};

#endif //PORFLOWMATERIALMASSFRACTIONBUILDER_H
