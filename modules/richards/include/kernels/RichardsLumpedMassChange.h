/*****************************************/
/* Written by andrew.wilkins@csiro.au    */
/* Please contact me if you make changes */
/*****************************************/

#ifndef RICHARDSLUMPEDMASSCHANGE
#define RICHARDSLUMPEDMASSCHANGE

#include "TimeKernel.h"
#include "Material.h"
#include "RichardsVarNames.h"
#include "RichardsDensity.h"
#include "RichardsSeff.h"
#include "RichardsSat.h"

// Forward Declarations
class RichardsLumpedMassChange;

template<>
InputParameters validParams<RichardsLumpedMassChange>();

/**
 * d(fluid mass in porespace)/dt with the fluid mass
 * being lumped to the nodes.  Usually this is better
 * to use than a non-lumped version because it prevents
 * unphysical oscillations.
 */
class RichardsLumpedMassChange : public TimeKernel
{
public:

  RichardsLumpedMassChange(const std::string & name,
                        InputParameters parameters);

protected:

  /**
   * This runs computeNodalValues for each porepressure variable
   * within the current element.  It also forms _ps_at_nodes and
   * _ps_old_at_nodes for the element, ready for insertion into seff_UO
   */
  void prepareNodalPressureValues();

  /**
   * Just runs prepareNodalPressureValues, then TimeKernel::computeResidual
   */
  virtual void computeResidual();

  virtual Real computeQpResidual();

  /**
   * Just runs prepareNodalPressureValues, then TimeKernel::computeJacobian
   */
  virtual void computeJacobian();

  virtual Real computeQpJacobian();

  /**
   * Just runs prepareNodalPressureValues, then TimeKernel::computeOffDiagJacobian
   */
  virtual void computeOffDiagJacobian(unsigned int jvar);

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
  MaterialProperty<Real> &_porosity;

  /// value of the porosity at the start of the timestep
  MaterialProperty<Real> &_porosity_old;

  /// The userobject that computes effective saturation (as a function of porepressure(s)) for this variable
  const RichardsSeff * _seff_UO;

  /// The userobject that computes saturation (as a function of effective saturation) for this variable
  const RichardsSat * _sat_UO;

  /// The userobject that computes fluid density (as a function of the porepressure)
  const RichardsDensity * _density_UO;

  /**
   * Holds the values of pressures at all the nodes of the element
   * Eg:
   * _ps_at_nodes[_pvar] is a pointer to this variable's nodal porepressure values
   * So: (*_ps_at_nodes[_pvar])[i] = _var.nodalSln()[i]
   */
  std::vector<VariableValue *> _ps_at_nodes;

  /// Holds the nodal values of pressures at timestep_begin, in same way as _ps_at_nodes
  std::vector<VariableValue *> _ps_old_at_nodes;

  /**
   * Holds the values of pressures at all the nodes of the element
   * This holds the same info as _ps_at_nodes, but in a different way.
   * Eg: _nodal_pp[_pvar]->nodalSln()[i] = (*_ps_at_nodes[_pvar])[i]
   * We call its computeNodalValues method in order to retrieve the nodal
   * porepressures from Moose
   */
  std::vector<MooseVariable *> _nodal_pp;

  /// holds nodal values of d(Seff)/dP_i
  std::vector<Real> _dseff;

};

#endif //RICHARDSLUMPEDMASSCHANGE
