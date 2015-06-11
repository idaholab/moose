/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef CHBULK_H
#define CHBULK_H

#include "KernelGrad.h"

//Forward Declarations
class CHBulk;

template<>
InputParameters validParams<CHBulk>();
/** This is the Cahn-Hilliard equation base class that implements the bulk or local energy term of the equation.
 *  See M.R. Tonks et al. / Computational Materials Science 51 (2012) 20–29 for more information.
 *  Note that the function computeGradDFDCons MUST be overridden in any kernel that inherits from
 *  CHBulk.  Use CHMath as an example of how this works.
 **/

class CHBulk : public KernelGrad
{
public:

  CHBulk(const std::string & name, InputParameters parameters);

protected:
  std::string _mob_name;
  std::string _Dmob_name;

  enum PFFunctionType
  {
    Jacobian,
    Residual
  };
  virtual RealGradient precomputeQpResidual();
  virtual RealGradient precomputeQpJacobian();
  virtual RealGradient computeGradDFDCons(PFFunctionType type) = 0;

  const MaterialProperty<Real> & _M;

private:
  bool _has_MJac;
  const MaterialProperty<Real> * _DM;
};

#endif //CHBULK_H
