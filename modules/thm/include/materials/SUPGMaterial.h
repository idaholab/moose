#ifndef SUPGMATERIAL_H
#define SUPGMATERIAL_H

#include "Material.h"
#include "DerivativeMaterialInterfaceTHM.h"

// Forward Declarations
class SUPGMaterial;
class Function;

template <>
InputParameters validParams<SUPGMaterial>();

/**
 * A material class for computing SUPG
 *
 * This was initially added for Shock-capturing stuff but is generally useful for SUPG as well if we
 * compute strong form residuals as material properties...
 */
class SUPGMaterial : public DerivativeMaterialInterfaceTHM<Material>
{
public:
  SUPGMaterial(const InputParameters & parameters);

protected:
  /**
   * The main Material interface to be specialized
   */
  virtual void computeProperties();

  /**
   * The Material base class gets the residualSetup() interface from
   * the SetupInterface class.  If we do nothing, this class is empty.
   * If we put something here, we should be able to do some setup
   * before each residual
   */
  virtual void residualSetup();

  /**
   * As above, but gets called once per timestep.
   */
  virtual void timestepSetup();

  // Material properties exposed to kernels using this material object

  // The shock capturing parameter
  MaterialProperty<Real> & _delta;

  // The advective flux matrix.  RealTensorValue is good for this...
  // it is always 3x3 as long as LIBMESH_DIM==3, and this is the
  // largest we need for the 1D non-isothermal problem.
  MaterialProperty<RealTensorValue> & _A;

  // A vector of strong residual values exposed as a material property
  MaterialProperty<RealVectorValue> & _R;

  // The columns of the stabilization matrix.  These are the y_k in the
  // compns notes.
  MaterialProperty<std::vector<RealVectorValue>> & _y;

  // The deritvative of the conserved variables wrt x, exposed as a material
  // property.
  MaterialProperty<RealVectorValue> & _dUdx;

  // Derivatives of the _A matrix with respect to the conserved variables.
  // There is one of these per dimension, per conserved variable, so in 1D
  // for the three equation model, there are 3.
  MaterialProperty<std::vector<RealTensorValue>> & _dA;

  // Conserved variables:

  // _arhoA =  [ rho,   constant-area equations
  //                 [ rhoA,  variable-area equations
  const VariableValue & _arhoA;

  // _arhouA = [ rhou,  constant-area equations
  //                 [ rhouA, variable-area equations
  const VariableValue & _arhouA;

  // _arhoEA =   [ rhoE,  constant-area equations
  //                 [ rhoEA, variable-area equations
  const VariableValue & _arhoEA;

  const VariableValue & _area;
  // The *actual* density, momentum, and total energy.  In the
  // constant-area equations, these are identical to _arhoA,
  // _arhouA, and _arhoEA defined above, in the
  // variable-area equations, these are the actual _rho, _rhou, and
  // _rhoE values.
  const VariableValue & _rho;
  const VariableValue & _rhou;
  const VariableValue & _rhoE;

  // Miscellaneous aux variables
  const VariableValue & _enthalpy;
  const VariableValue & _temperature;
  bool _has_heat_transfer;
  const MaterialProperty<Real> & _Hw;
  const VariableValue & _vel;

  const MaterialProperty<Real> & _p;
  const MaterialProperty<Real> & _dp_drhoA;
  const MaterialProperty<Real> & _dp_drhouA;
  const MaterialProperty<Real> & _dp_drhoEA;

  const VariableValue & _D_h;

  // Heat flux perimeter
  const VariableValue & _P_hf;

  // Time derivative values for dependent variables
  const VariableValue & _ddt_arhoA;
  const VariableValue & _ddt_arhouA;
  const VariableValue & _ddt_arhoEA;

  // Gradients of conserved variables:

  // _grad_arhoA =  [ grad(rho),   constant-area equations
  //                      [ grad(rhoA),  variable-area equations
  const VariableGradient & _grad_arhoA;

  // _grad_arhouA = [ grad(rhou),  constant-area equations
  //                      [ grad(rhouA), variable-area equations
  const VariableGradient & _grad_arhouA;

  // _grad_arhoEA =   [ grad(rhoE),  constant-area equations
  //                      [ grad(rhoEA), variable-area equations
  const VariableGradient & _grad_arhoEA;

  // Gradients of aux variables
  const VariableGradient & _grad_area;

  /// The direction of the pipe
  const MaterialProperty<RealVectorValue> & _dir;
  /// Gravitational acceleration vector
  const RealVectorValue & _gravity_vector;
  // Required parameters
  const MaterialProperty<Real> & _f_D;
  /// Wall temperature
  const MaterialProperty<Real> & _T_wall;

  // Optional parameters
  // If using shock capturing, how many Newton iterations should we do before
  // freezing delta?
  unsigned _n_iterations_before_freezing_delta;

  // By default true. If false during the call to computeProperties(),
  // the shock-capturing parameter is not recomputed, but instead pulled
  // from values cached locally in this class.
  //
  // WARNING: USING THIS IS NOT THREAD-SAFE!
  bool _recompute_delta;

  // NOT THREAD-SAFE!  This data structure is used to cache delta values
  // at the quadrature points as they are computed.  Once delta is frozen,
  // the cached values will be used instead of recomputing new delta values.
  std::map<unsigned, std::vector<Real>> _cached_delta;
};

#endif // SUPGMATERIAL_H
