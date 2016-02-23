/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/


#ifndef RICHARDSBOREHOLE_H
#define RICHARDSBOREHOLE_H

// Moose Includes
#include "DiracKernel.h"
#include "Function.h"
#include "RichardsSumQuantity.h"
#include "RichardsVarNames.h"
#include "RichardsDensity.h"
#include "RichardsRelPerm.h"
#include "RichardsSeff.h"


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
  RichardsBorehole(const InputParameters & parameters);

  /**
   * Add Dirac Points to the borehole
   */
  virtual void addPoints();

  /**
   * Computes the residual.  This just
   * calls prepareNodalValues, if _fully_upwind
   * then calls DiracKernel::computeResidual
   */
  virtual void computeResidual();

  /**
   * Computes the Qp residual
   */
  virtual Real computeQpResidual();


  /**
   * Computes the Jacobian.  This just
   * calls prepareNodalValues, if _fully_upwind
   * then calls DiracKernel::computeJacobian
   */
  virtual void computeJacobian();

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

  /// Whether to use full upwinding
  bool _fully_upwind;

  /// Defines the richards variables in the simulation
  const RichardsVarNames & _richards_name_UO;

  /// number of richards variables
  unsigned int _num_p;

  /// The moose internal variable number of the richards variable of this Dirac Kernel
  unsigned int _pvar;

  /// user object defining the density.  Only used if _fully_upwind = true
  const RichardsDensity * _density_UO;

  /// user object defining the effective saturation.  Only used if _fully_upwind = true
  const RichardsSeff * _seff_UO;

  /// user object defining the relative permeability.  Only used if _fully_upwind = true
  const RichardsRelPerm * _relperm_UO;

  /// number of nodes in this element.  Only used if _fully_upwind = true
  unsigned int _num_nodes;

  /**
   * nodal values of mobility = density*relperm/viscosity
   * These are used if _fully_upwind = true
   */
  std::vector<Real> _mobility;

  /**
   * d(_mobility)/d(variable_ph)  (variable_ph is the variable for phase=ph)
   * These are used in the jacobian calculations if _fully_upwind = true
   */
  std::vector<std::vector<Real> > _dmobility_dv;



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

  /// borehole length.  Note this is only used if there is only one borehole point
  Real _borehole_length;

  /// borehole direction.  Note this is only used if there is only one borehole point
  RealVectorValue _borehole_direction;

  /// fluid porepressure (or porepressures in case of multiphase)
  const MaterialProperty<std::vector<Real> > & _pp;

  /// d(porepressure_i)/d(variable_j)
  const MaterialProperty<std::vector<std::vector<Real> > > & _dpp_dv;

  /// fluid viscosity
  const MaterialProperty<std::vector<Real> > & _viscosity;

  /// material permeability
  const MaterialProperty<RealTensorValue> & _permeability;

  /// deriviatves of Seff wrt variables
  const MaterialProperty<std::vector<std::vector<Real> > > & _dseff_dv;

  /// relative permeability
  const MaterialProperty<std::vector<Real> > & _rel_perm;

  /// d(relperm_i)/d(variable_j)
  const MaterialProperty<std::vector<std::vector<Real> > > & _drel_perm_dv;

  /// fluid density
  const MaterialProperty<std::vector<Real> > & _density;

  /// d(density_i)/d(variable_j)
  const MaterialProperty<std::vector<std::vector<Real> > > & _ddensity_dv;

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

  /// whether using the _dseff_val, _relperm_val, etc (otherwise values from RichardsMaterial are used)
  bool _using_coupled_vars;

  /**
   * Holds the values of pressures at all the nodes of the element
   * Only used if _fully_upwind = true
   * Eg:
   * _ps_at_nodes[_pvar] is a pointer to this variable's nodal porepressure values
   * So: (*_ps_at_nodes[_pvar])[i] = _var.nodalSln()[i] = porepressure of pressure-variable _pvar at node i
   */
  std::vector<const VariableValue *> _ps_at_nodes;


  /// reads a space-separated line of floats from ifs and puts in myvec
  bool parseNextLineReals(std::ifstream & ifs, std::vector<Real> & myvec);

  /**
   * Calculates Peaceman's form of the borehole well constant
   * Z Chen, Y Zhang, Well flow models for various numerical methods, Int J Num Analysis and Modeling, 3 (2008) 375-388
   */
  Real wellConstant(const RealTensorValue & perm, const RealTensorValue & rot, const Real & half_len, const Elem * ele, const Real & rad);

  /// calculates the nodal values of pressure, mobility, and derivatives thereof
  void prepareNodalValues();


  /**
   * Calculates Jacobian
   * @param wrt_num differentiate the residual wrt this Richards variable
   */
  Real jac(unsigned int wrt_num);

};

#endif //RICHARDSBOREHOLE_H
