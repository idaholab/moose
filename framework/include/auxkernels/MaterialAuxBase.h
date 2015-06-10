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

#ifndef MATERIALAUXBASE_H
#define MATERIALAUXBASE_H

// MOOSE includes
#include "AuxKernel.h"

// Forward declarations
template<typename T>
class MaterialAuxBase;

template<>
InputParameters validParams<MaterialAuxBase<Real> >();

template<>
InputParameters validParams<MaterialAuxBase<RealVectorValue> >();

template<>
InputParameters validParams<MaterialAuxBase<RealTensorValue> >();

template<>
InputParameters validParams<MaterialAuxBase<DenseMatrix<Real> > >();

/**
 * A base class for the various Material related AuxKernal objects
 */
template<typename T>
class MaterialAuxBase : public AuxKernel
{
public:

  /**
   * Class constructor
   * @param name Name of the AuxKernel
   * @param parameters The input parameters for this object
   */
  MaterialAuxBase(const std::string & name, InputParameters parameters);

  /**
   * Class destructor
   */
  virtual ~MaterialAuxBase(){}

protected:

  /// Reference to the material property for this AuxKernel
  const MaterialProperty<T> & _prop;

  // Value to be added to the material property
  const Real _factor;

  // Multiplier for the material property
  const Real _offset;

};

template<typename T>
MaterialAuxBase<T>::MaterialAuxBase(const std::string & name, InputParameters parameters) :
    AuxKernel(name, parameters),
    _prop(getMaterialProperty<T>(getParam<std::string>("property"))),
    _factor(getParam<Real>("factor")),
    _offset(getParam<Real>("offset"))
{
}

#endif //MATERIALAUXBASE_H
