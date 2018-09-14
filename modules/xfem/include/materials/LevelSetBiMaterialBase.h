/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef LEVELSETBIMATERIALBASE_H
#define LEVELSETBIMATERIALBASE_H

#include "Material.h"

// Forward Declarations
class LevelSetBiMaterialBase;
class XFEM;

template <>
InputParameters validParams<LevelSetBiMaterialBase>();

/**
 * Base class for switching between materials in a bi-material system where the interface is defined
 * by a level set function.
 */
class LevelSetBiMaterialBase : public Material
{
public:
  LevelSetBiMaterialBase(const InputParameters & parameters);

protected:
  virtual void computeProperties();
  virtual void computeQpProperties();

  /**
   * assign the material properties for the negative level set region.
   */
  virtual void assignQpPropertiesForLevelSetNegative() = 0;

  /**
   * assign the material properties for the positive level set region.
   */
  virtual void assignQpPropertiesForLevelSetPositive() = 0;

  /// global material properties
  std::string _base_name;

  /// Property name
  std::string _prop_name;

  /// shared pointer to XFEM
  std::shared_ptr<XFEM> _xfem;

  /// The variable number of the level set variable we are operating on
  const unsigned int _level_set_var_number;

  /// system reference
  const System & _system;

  /// the subproblem solution vector
  const NumericVector<Number> * _solution;

  /// use the positive level set region's material properties
  bool _use_positive_property;
};

#endif // LEVELSETBIMATERIALBASE_H
