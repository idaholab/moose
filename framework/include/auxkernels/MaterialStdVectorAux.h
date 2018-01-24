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
