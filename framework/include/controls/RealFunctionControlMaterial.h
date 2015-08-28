/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#ifndef REALFUNCTIONCONTROLMATERIAL_H
#define REALFUNCTIONCONTROLMATERIAL_H

// MOOSE includes
#include "ControlMaterial.h"
#include "Function.h"

// Forward declarations
class RealFunctionControlMaterial;

template<>
InputParameters validParams<RealFunctionControlMaterial>();

/**
 *
 */
class RealFunctionControlMaterial : public ControlMaterial
{
public:

  /**
   * Class constructor
   * @param parameters
   */
  RealFunctionControlMaterial(const InputParameters & parameters);

  /**
   * Class destructor
   */
  virtual ~RealFunctionControlMaterial(){}

  virtual void computeQpProperties();


private:

  Function & _function;

  MaterialProperty<Real> & _control_prop;

};



#endif // REALFUNCTIONCONTROLMATERIAL_H
