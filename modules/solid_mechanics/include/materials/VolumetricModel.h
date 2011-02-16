/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2011 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#ifndef VOLUMETRICMODEL_H
#define VOLUMETRICMODEL_H

#include "Material.h"

class ColumnMajorMatrix;

class VolumetricModel : public Material
{
public:
  VolumetricModel( const std::string & name,
                   InputParameters & parameters );
  ~VolumetricModel();

  virtual void modifyStrain(const unsigned int qp,
                            ColumnMajorMatrix & strain_increment) = 0;

};

#endif // VOLUMETRICMODEL_H
