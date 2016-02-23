/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/


#ifndef Q2PBOREHOLE_H
#define Q2PBOREHOLE_H

// Moose Includes
#include "DiracKernel.h"
#include "Function.h"
#include "RichardsSumQuantity.h"
#include "RichardsDensity.h"
#include "RichardsRelPerm.h"


class Q2PBorehole;

template<>
InputParameters validParams<Q2PBorehole>();

/**
 * Approximates a borehole by a sequence of Dirac Points.
 * This is for use by a Q2P model.
 */
class Q2PBorehole : public DiracKernel
{
public:

  /**
   * Creates a new Q2PBorehole
   * This sets all the variables, but also
   * reads the file containing the lines of the form
   * radius x y z
   * that defines the borehole geometry.
   * It also calculates segment-lengths and rotation matrices
   * needed for computing the borehole well constant
   */
  Q2PBorehole(const InputParameters & parameters);

  /**
   * Add Dirac Points to the borehole
   */
  virtual void addPoints();

  /**
   * Computes the residual.  This just
   * calls prepareNodalValues
   * then calls DiracKernel::computeResidual
   */
  virtual void computeResidual();

  /**
   * Computes the Qp residual
   */
  virtual Real computeQpResidual();


  /**
   * Computes the Jacobian.  This just
   * calls prepareNodalValues
   * then calls DiracKernel::computeJacobian
   */
  virtual void computeJacobian();

  /**
   * Computes the diagonal part of the jacobian
   */
  virtual Real computeQpJacobian();

  /**
   * Computes the off-diagonal part of the jacobian
   */
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

protected:

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

  /// number of nodes in this element.
  unsigned int _num_nodes;

  /// nodal porepressure
  std::vector<Real> _pp;

  /// nodal saturation
  std::vector<Real> _sat;

  /// nodal mobility
  std::vector<Real> _mobility;

  /// nodal d(mobility)/d(porepressure)
  std::vector<Real> _dmobility_dp;

  /// nodal d(mobility)/d(saturation)
  std::vector<Real> _dmobility_ds;

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
   * @param jvar differentiate the residual wrt this variable
   */
  Real jac(unsigned int jvar);

};

#endif //Q2PBOREHOLE_H
