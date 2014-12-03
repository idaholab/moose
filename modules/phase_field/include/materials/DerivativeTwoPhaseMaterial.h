#ifndef DERIVATIVETWOPHASEMATERIAL_H
#define DERIVATIVETWOPHASEMATERIAL_H

#include "DerivativeBaseMaterial.h"

// Forward Declarations
class DerivativeTwoPhaseMaterial;

template<>
InputParameters validParams<DerivativeTwoPhaseMaterial>();

/**
 * DerivativeMaterial child class to evaluate a parsed function for the
 * free energy and automatically provide all derivatives.
 * This requires the autodiff patch (https://github.com/libMesh/libmesh/pull/238)
 * to Function Parser in libmesh.
 */
class DerivativeTwoPhaseMaterial : public DerivativeBaseMaterial
{
public:
  DerivativeTwoPhaseMaterial(const std::string & name,
                             InputParameters parameters);

protected:
  static InputParameters addPhiToArgs(InputParameters);

  virtual unsigned int expectedNumArgs();

  virtual Real computeF();
  virtual Real computeDF(unsigned int);
  virtual Real computeD2F(unsigned int, unsigned int);

  virtual Real h(Real phi);
  virtual Real dh(Real phi);
  virtual Real d2h(Real phi);
  virtual Real g(Real phi);
  virtual Real dg(Real phi);
  virtual Real d2g(Real phi);

  /// A-phase derivative material name
  std::string _fa_name;
  /// B-phase derivative material name
  std::string _fb_name;

  /// Phase parameter (0=A-phase, 1=B-phase)
  VariableValue & _phi;

  /// index of the phi in _args (after it is appended by addPhiToArgs())
  unsigned int _phi_id;
  unsigned int _nfargs;

  /// Phase transformatuion energy barrier
  Real _W;

  /// Function value of the A and B phase.
  const MaterialProperty<Real> * _prop_Fa, * _prop_Fb;

  /// Derivatives of Fa and Fb with respect to arg[i]
  std::vector<const MaterialProperty<Real> *> _prop_dFa, _prop_dFb;

  /// Second derivatives of Fa and Fb.
  std::vector<std::vector<const MaterialProperty<Real> *> > _prop_d2Fa, _prop_d2Fb;

  /// Third derivatives of Fa and Fb.
  std::vector<std::vector<std::vector<const MaterialProperty<Real> *> > > _prop_d3Fa, _prop_d3Fb;
};

#endif // DERIVATIVETWOPHASEMATERIAL_H
