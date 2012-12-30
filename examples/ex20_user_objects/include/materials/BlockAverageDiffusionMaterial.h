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

#ifndef BLOCKAVERAGEDIFFUSIONMATERIAL_H
#define BLOCKAVERAGEDIFFUSIONMATERIAL_H

#include "Material.h"
#include "BlockAverageValue.h"

//Forward Declarations
class BlockAverageDiffusionMaterial;

template<>
InputParameters validParams<BlockAverageDiffusionMaterial>();

class BlockAverageDiffusionMaterial : public Material
{
public:
  BlockAverageDiffusionMaterial(const std::string & name,
                  InputParameters parameters);

protected:
  virtual void computeQpProperties();

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

#endif //BLOCKAVERAGEDIFFUSIONMATERIAL_H
