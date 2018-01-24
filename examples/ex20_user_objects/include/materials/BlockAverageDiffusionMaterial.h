//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef BLOCKAVERAGEDIFFUSIONMATERIAL_H
#define BLOCKAVERAGEDIFFUSIONMATERIAL_H

#include "Material.h"
#include "BlockAverageValue.h"

// Forward Declarations
class BlockAverageDiffusionMaterial;

template <>
InputParameters validParams<BlockAverageDiffusionMaterial>();

class BlockAverageDiffusionMaterial : public Material
{
public:
  BlockAverageDiffusionMaterial(const InputParameters & parameters);

protected:
  virtual void computeQpProperties() override;

private:
  /**
   * This is the member reference that will hold the computed values
   * for the Real value property in this class.
   */
  MaterialProperty<Real> & _diffusivity;

  /**
   * A member reference that will hold onto a UserObject
   * of type BlockAverageValue for us to be able to query
   * the average value of a variable on each block.
   *
   * NOTE: UserObject references are _const_!
   */
  const BlockAverageValue & _block_average_value;
};

#endif // BLOCKAVERAGEDIFFUSIONMATERIAL_H
