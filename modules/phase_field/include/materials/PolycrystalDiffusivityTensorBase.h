/****************************************************************/
/*                  DO NOT MODIFY THIS HEADER                   */
/*                           Marmot                             */
/*                                                              */
/*            (c) 2017 Battelle Energy Alliance, LLC            */
/*                     ALL RIGHTS RESERVED                      */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*             Under Contract No. DE-AC07-05ID14517             */
/*             With the U. S. Department of Energy              */
/*                                                              */
/*             See COPYRIGHT for full restrictions              */
/****************************************************************/

#ifndef POLYCRYSTALDIFFUSIVITYTENSORBASE_H
#define POLYCRYSTALDIFFUSIVITYTENSORBASE_H

#include "Material.h"
#include "DerivativeMaterialInterface.h"

// Forward Declarations
class PolycrystalDiffusivityTensorBase;

template <>
InputParameters validParams<PolycrystalDiffusivityTensorBase>();

/**
 * Generates a diffusion tensor to distinguish between the bulk, grain boundary,
 * and surface diffusion rates.
 */
class PolycrystalDiffusivityTensorBase : public DerivativeMaterialInterface<Material>
{
public:
  PolycrystalDiffusivityTensorBase(const InputParameters & parameters);

protected:
  virtual void computeProperties();
  const VariableValue & _T;
  std::vector<const VariableValue *> _vals;
  std::vector<const VariableGradient *> _grad_vals;
  const VariableValue & _c;
  const VariableGradient & _grad_c;
  VariableName _c_name;

  MaterialProperty<RealTensorValue> & _D;
  MaterialProperty<RealTensorValue> & _dDdc;

  Real _D0;
  Real _Em;
  Real _s_index;
  Real _gb_index;
  Real _b_index;
  Real _Dbulk;

  const Real _kb;
  const unsigned int _op_num;
};

#endif // POLYCRYSTALDIFFUSIVITYTENSORBASE_H
