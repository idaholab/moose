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

#ifndef MATERIALDATA_H
#define MATERIALDATA_H

//MOOSE includes
#include "Moose.h"
#include "MooseArray.h"
#include "Material.h"

//libMesh includes
#include "transient_system.h"

//Forward Declarations
class MooseSystem;

class MaterialData
{
public:
  MaterialData(MooseSystem & moose_system);

  MooseSystem & _moose_system;

  std::map<std::string, PropertyValue *> _props;
  std::map<std::string, PropertyValue *> _props_old;
  std::map<std::string, PropertyValue *> _props_older;
};
#endif //MATERIALDATA_H
