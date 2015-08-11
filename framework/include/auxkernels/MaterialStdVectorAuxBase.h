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

#ifndef MATERIALSTDVECTORAUXBASE_H
#define MATERIALSTDVECTORAUXBASE_H

// MOOSE includes
#include "MaterialAuxBase.h"

// Forward declarations
template<typename T = Real>
class MaterialStdVectorAuxBase;

template<>
InputParameters validParams<MaterialStdVectorAuxBase<> >();
/**
 * A base class for the various Material related AuxKernal objects
 */
template<typename T>
class MaterialStdVectorAuxBase : public MaterialAuxBase<std::vector<T> >
{
public:
  MaterialStdVectorAuxBase(const InputParameters & parameters);

  virtual ~MaterialStdVectorAuxBase(){}

  virtual Real computeValue();

protected:
  /// index of the vecor element
  unsigned int _index;
};

template<typename T>
MaterialStdVectorAuxBase<T>::MaterialStdVectorAuxBase(const InputParameters & parameters) :
    MaterialAuxBase<std::vector<T> >(parameters),
    _index(this->template getParam<unsigned int>("index"))
{
}

template<typename T>
Real
MaterialStdVectorAuxBase<T>::computeValue()
{
  mooseAssert(this->_prop[this->_qp].size() > _index, "MaterialStdVectorRealGradientAux: You chose to extract component " << _index << " but your Material property only has size " << this->_prop[this->_qp].size());
  return MaterialAuxBase<std::vector<T> >::computeValue();
}

#endif //MATERIALSTDVECTORAUXBASE_H
