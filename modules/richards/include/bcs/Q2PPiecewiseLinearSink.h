/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef Q2PPIECEWISELINEARSINK
#define Q2PPIECEWISELINEARSINK

#include "IntegratedBC.h"
#include "LinearInterpolation.h"
#include "Function.h"
#include "RichardsDensity.h"
#include "RichardsRelPerm.h"

// Forward Declarations
class Q2PPiecewiseLinearSink;

template <>
InputParameters validParams<Q2PPiecewiseLinearSink>();

/**
 * Applies a fully-upwinded flux sink to a boundary
 * The sink is a piecewise linear function of
 * porepressure at the quad points.
 * This is specified by _sink_func.
 * In addition, this sink can be multiplied by:
 *  (1) the relative permeability of the fluid at the quad point.
 *  (2) perm_nn*density/viscosity, where perm_nn is the
 *      permeability tensor projected in the normal direction.
 *  (3) a Function (which can be time-dependent, for instance)
 * This is for use in Q2P models
 */
class Q2PPiecewiseLinearSink : public IntegratedBC
{
public:
  Q2PPiecewiseLinearSink(const InputParameters & parameters);

protected:
  virtual void computeResidual();

  virtual Real computeQpResidual();

  virtual void computeJacobian();

  virtual Real computeQpJacobian();

  virtual void computeJacobianBlock(unsigned int jvar);

  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

  /// whether to multiply the sink flux by permeability*density/viscosity
  bool _use_mobility;

  /// whether to multiply the sink flux by relative permeability
  bool _use_relperm;

  /// piecewise-linear function of porepressure (this defines the strength of the sink)
  LinearInterpolation _sink_func;

  /// sink flux gets multiplied by this function
  Function & _m_func;

  /// fluid density
  const RichardsDensity & _density;

  /// fluid relative permeability
  const RichardsRelPerm & _relperm;

  /// the other variable in the 2-phase system (this is saturation if Variable=porepressure, and viceversa)
  const VariableValue & _other_var_nodal;

  /// the variable number of the other variable
  unsigned int _other_var_num;

  /// whether the Variable for this BC is porepressure or not
  bool _var_is_pp;

  /// viscosity
  Real _viscosity;

  /// permeability
  const MaterialProperty<RealTensorValue> & _permeability;

  /// number of nodes in this element.
  unsigned int _num_nodes;

  /// nodal values of porepressure
  std::vector<Real> _pp;

  /// nodal values of saturation
  std::vector<Real> _sat;

  /// nodal values of fluid density
  std::vector<Real> _nodal_density;

  /// d(_nodal_density)/d(porepressure)
  std::vector<Real> _dnodal_density_dp;

  /// nodal values of relative permeability
  std::vector<Real> _nodal_relperm;

  /// d(_nodal_relperm)/d(saturation)
  std::vector<Real> _dnodal_relperm_ds;

  /// calculates the nodal values of pressure, mobility, and derivatives thereof
  void prepareNodalValues();

  /// derivative of residual wrt the wrt_num variable
  Real jac(unsigned int wrt_num);
};

#endif // Q2PPIECEWISELINEARSINK
