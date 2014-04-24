/*****************************************/
/* Written by andrew.wilkins@csiro.au    */
/* Please contact me if you make changes */
/*****************************************/

#ifndef RICHARDSPIECEWISELINEARSINK
#define RICHARDSPIECEWISELINEARSINK

#include "IntegratedBC.h"
#include "LinearInterpolation.h"
#include "RichardsPorepressureNames.h"
#include "Function.h"

// Forward Declarations
class RichardsPiecewiseLinearSink;

template<>
InputParameters validParams<RichardsPiecewiseLinearSink>();

/**
 * Applies a flux sink to a boundary
 * The sink is a piecewise linear function of
 * porepressure (the "variable") at the quad points.
 * This is specified by _sink_func.
 * In addition, this sink can be multiplied by:
 *  (1) the relative permeability of the fluid at the quad point.
 *  (2) perm_nn*density/viscosity, where perm_nn is the
 *      permeability tensor projected in the normal direction.
 *  (3) a Function (which can be time-dependent, for instance)
 */
class RichardsPiecewiseLinearSink : public IntegratedBC
{
public:

  RichardsPiecewiseLinearSink(const std::string & name,
                        InputParameters parameters);

protected:
  virtual Real computeQpResidual();

  virtual Real computeQpJacobian();

  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

  /// whether to multiply the sink flux by permeability*density/viscosity
  bool _use_mobility;

  /// whether to multiply the sink flux by relative permeability
  bool _use_relperm;

  /// piecewise-linear function of porepressure (this defines the strength of the sink)
  LinearInterpolation _sink_func;

  /// sink flux gets multiplied by this function
  Function & _m_func;

  /// holds info about the names and values of porepressures in the simulation
  const RichardsPorepressureNames & _pp_name_UO;

  /// the moose internal variable number corresponding to the porepressure of this sink flux
  unsigned int _pvar;

  /// viscosity (only the _pvar component is used)
  MaterialProperty<std::vector<Real> > &_viscosity;

  /// permeability
  MaterialProperty<RealTensorValue> & _permeability;

  /**
   * derivative of effective saturation wrt porepressure variables
   * only _dseff[_pvar][i] is used for i being all porepressure variables
   */
  MaterialProperty<std::vector<std::vector<Real> > > &_dseff;

  /// relative permeability (only the _pvar component is used)
  MaterialProperty<std::vector<Real> > &_rel_perm;

  /// derivative of relative permeability wrt effective saturation (only the _pvar component is used)
  MaterialProperty<std::vector<Real> > &_drel_perm;

  /// fluid density (only the _pvar component is used)
  MaterialProperty<std::vector<Real> > &_density;

  /// derivative of fluid density wrt porepressure (only the _pvar component is used)
  MaterialProperty<std::vector<Real> > &_ddensity;


};

#endif //RichardsPiecewiseLinearSink
