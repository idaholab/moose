/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef CHBULK_H
#define CHBULK_H

#include "KernelGrad.h"
#include "JvarMapInterface.h"
#include "DerivativeMaterialInterface.h"

//Forward Declarations
class CHBulk;

template<>
InputParameters validParams<CHBulk>();
/** This is the Cahn-Hilliard equation base class that implements the bulk or local energy term of the equation.
 *  See M.R. Tonks et al. / Computational Materials Science 51 (2012) 20–29 for more information.
 *  Note that the function computeGradDFDCons MUST be overridden in any kernel that inherits from
 *  CHBulk.  Use CHMath as an example of how this works.
 **/

class CHBulk : public DerivativeMaterialInterface<JvarMapInterface<KernelGrad> >
{
public:

  CHBulk(const std::string & name, InputParameters parameters);

protected:

  virtual RealGradient precomputeQpResidual();
  virtual RealGradient precomputeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

  enum PFFunctionType
  {
    Jacobian,
    Residual
  };

  virtual RealGradient computeGradDFDCons(PFFunctionType type) = 0;

  const std::string _mob_name;
  const MaterialProperty<Real> & _M;
  const MaterialProperty<Real> & _dMdc;

  std::vector<const MaterialProperty<Real> *> _dMdarg;
};

#endif //CHBULK_H
