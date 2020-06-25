//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "TimeKernel.h"
#include "Material.h"
#include "RichardsVarNames.h"
#include "RichardsDensity.h"
#include "RichardsSeff.h"
#include "RichardsSat.h"

// Forward Declarations

/**
 * d(fluid mass in porespace)/dt with the fluid mass
 * being lumped to the nodes.  Usually this is better
 * to use than a non-lumped version because it prevents
 * unphysical oscillations.
 */
class RichardsLumpedMassChange : public TimeKernel
{
public:
  static InputParameters validParams();

  RichardsLumpedMassChange(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual();

  virtual Real computeQpJacobian();

  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

  /**
   * holds info regarding the names of the Richards variables
   * and methods for extracting values of these variables
   */
  const RichardsVarNames & _richards_name_UO;

  /// number of richards variables
  unsigned int _num_p;

  /**
   * the index of this variable in the list of Richards variables
   * held by _richards_name_UO.  Eg
   * if richards_vars = 'pwater pgas poil' in the _richards_name_UO
   * and this kernel has variable = pgas, then _pvar = 1
   * This is used to index correctly into seff_UO, sat_UO, density_UO, etc.
   */
  unsigned int _pvar;

  /// current value of the porosity
  const MaterialProperty<Real> & _porosity;

  /// value of the porosity at the start of the timestep
  const MaterialProperty<Real> & _porosity_old;

  /// The userobject that computes effective saturation (as a function of porepressure(s)) for this variable
  const RichardsSeff & _seff_UO;

  /// The userobject that computes saturation (as a function of effective saturation) for this variable
  const RichardsSat & _sat_UO;

  /// The userobject that computes fluid density (as a function of the porepressure)
  const RichardsDensity & _density_UO;

  /**
   * Holds the values of pressures at all the nodes of the element
   * Eg:
   * _ps_at_nodes[_pvar] is a pointer to this variable's nodal porepressure values
   * So: (*_ps_at_nodes[_pvar])[i] = _var.dofValues()[i]
   */
  std::vector<const VariableValue *> _ps_at_nodes;

  /// Holds the nodal values of pressures at timestep_begin, in same way as _ps_at_nodes
  std::vector<const VariableValue *> _ps_old_at_nodes;

  /// holds nodal values of d(Seff)/dP_i
  std::vector<Real> _dseff;
};
