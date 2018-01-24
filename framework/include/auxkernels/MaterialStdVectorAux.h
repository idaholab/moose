//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef MATERIALSTDVECTORAUX_H
#define MATERIALSTDVECTORAUX_H

#include "MaterialStdVectorAuxBase.h"

// Forward declarations
class MaterialStdVectorAux;

template <>
InputParameters validParams<MaterialStdVectorAux>();

/**
 * AuxKernel for outputting a std::vector material-property component to an AuxVariable
 */
class MaterialStdVectorAux : public MaterialStdVectorAuxBase<Real>
{
public:
  /**
   * Class constructor
   * @param parameters The input parameters for this object
   */
  MaterialStdVectorAux(const InputParameters & parameters);

protected:
  virtual Real getRealValue() override;

  /// whether or not selected_qp has been set
  const bool _has_selected_qp;

  /// The std::vector will be evaluated at this quadpoint only
  const unsigned int _selected_qp;
};

#endif // MATERIALSTDVECTORAUX_H
