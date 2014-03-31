/*****************************************/
/* Written by andrew.wilkins@csiro.au    */
/* Please contact me if you make changes */
/*****************************************/

#ifndef RICHARDSBOREHOLE_H
#define RICHARDSBOREHOLE_H

// Moose Includes
#include "DiracKernel.h"
#include "Function.h"
#include "RichardsSumQuantity.h"
#include "RichardsPorepressureNames.h"

class RichardsBorehole;

template<>
InputParameters validParams<RichardsBorehole>();

/**
 * Approximates a borehole by a sequence of Dirac Points
 */
class RichardsBorehole : public DiracKernel
{
public:

  /**
   * Creates a new RichardsBorehole
   * This sets all the variables, but also
   * reads the file containing the lines of the form
   * radius x y z
   * that defines the borehole geometry.
   * It also calculates segment-lengths and rotation matrices
   * needed for computing the borehole well constant
   */
  RichardsBorehole(const std::string & name, InputParameters parameters);

  /**
   * Add Dirac Points to the borehole
   * If !_mesh_adaptivity then the containing element for
   * each point gets cached to speed up computations
   * at later time steps
   */
  virtual void addPoints();

  /**
   * Computes the residual
   */
  virtual Real computeQpResidual();

  /**
   * Computes the diagonal part of the jacobian
   */
  virtual Real computeQpJacobian();

  /**
   * Computes the off-diagonal part of the jacobian
   * Note: at March2014 this is never called since
   * moose does not have this functionality.  Hence
   * as of March2014 this has never been tested.
   */
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

protected:

  /// Checks rotation matrices are correct
  bool _debug_things;

  /// Derivative of Seff wrt pressure.  If entered then the value from RichardsMaterial is not used.
  VariableValue * _dseff_val;

  /// Relative permeability.  If entered then the value from RichardsMaterial is not used.
  VariableValue * _relperm_val;

  /// Derivative of relative perm wrt Seff.  If entered then the value from RichardsMaterial is not used.
  VariableValue * _drelperm_val;

  /// Fluid density.  If entered then the value from RichardsMaterial is not used.
  VariableValue * _density_val;

  /// Derivative of density wrt pressure.  If entered then the value from RichardsMaterial is not used.
  VariableValue * _ddensity_val;

  /// Defines the porepressure variables in the simulation
  const RichardsPorepressureNames & _pp_name_UO;

  /// The moose internal variable number of the porepresure of this Dirac Kernel
  unsigned int _pvar;

  /**
   * If positive then the borehole acts as a sink (producion well) for porepressure > borehole pressure, and does nothing otherwise
   * If negative then the borehole acts as a source (injection well) for porepressure < borehole pressure, and does nothing otherwise
   * The flow rate to/from the borehole is multiplied by |character|, so usually character = +/- 1
   */
  Function & _character;

  /// bottomhole pressure of borehole
  Real _p_bot;

  /// unit weight of fluid in borehole (for calculating bottomhole pressure at each Dirac Point)
  RealVectorValue _unit_weight;

  /// borehole constant
  Real _re_constant;

  /// well constant
  Real _well_constant;

  /// whether mesh adaptivity is used (if not then caching of elemental info is used to speed up addPoints)
  bool _mesh_adaptivity;

  /// fluid viscosity
  MaterialProperty<std::vector<Real> > &_viscosity;

  /// material permeability
  MaterialProperty<RealTensorValue> & _permeability;

  /// deriviatves of Seff wrt pressures
  MaterialProperty<std::vector<std::vector<Real> > > &_dseff;

  /// relative permeability
  MaterialProperty<std::vector<Real> > &_rel_perm;

  /// deriv of rel perm wrt Seff
  MaterialProperty<std::vector<Real> > &_drel_perm;

  /// fluid density
  MaterialProperty<std::vector<Real> > &_density;

  /// derivative of density wrt pressure
  MaterialProperty<std::vector<Real> > &_ddensity;

  /**
   * This is used to hold the total fluid flowing into the borehole
   * Hence, it is positive for production wells where fluid is flowing
   * from porespace into the borehole and removed from the model
   */
  RichardsSumQuantity & _total_outflow_mass;

  /**
   * File defining the geometry of the borehole.   Each row has format
   * radius x y z
   * and the list of such points defines a polyline that is the borehole
   */
  std::string _point_file;

  /// radii of the borehole
  std::vector<Real> _rs;

  /// x points of the borehole
  std::vector<Real> _xs;

  /// y points of the borehole
  std::vector<Real> _ys;

  /// z points of borehole
  std::vector<Real> _zs;

  /// the bottom point of the borehole (where bottom_pressure is defined)
  Point _bottom_point;

  /// 0.5*(length of polyline segments between points)
  std::vector<Real> _half_seg_len;

  /// rotation matrix used in well_constant calculation
  std::vector<RealTensorValue> _rot_matrix;

  /// the cache of elements containing the Dirac Points
  std::vector<const Elem *> _elemental_info;

  /// whether _elemental_info has been constructed
  bool _have_constructed_elemental_info;

  /// whether using the _dseff_val, _relperm_val, etc (otherwise values from RichardsMaterial are used)
  bool _using_coupled_vars;

  /// reads a space-separated line of floats from ifs and puts in myvec
  bool parseNextLineReals(std::ifstream & ifs, std::vector<Real> &myvec);

  /// provides a rotation matrix that will rotate the vector v2 to the z axis (the "2" direction)
  RealTensorValue rotVecToZ(RealVectorValue v2);

  /**
   * Calculates Peaceman's form of the borehole well constant
   * Z Chen, Y Zhang, Well flow models for various numerical methods, Int J Num Analysis and Modeling, 3 (2008) 375-388
   */
  Real wellConstant(const RealTensorValue & perm, const RealTensorValue & rot, const Real & half_len, const Elem * ele, const Real & rad);
};

#endif //RICHARDSBOREHOLE_H
