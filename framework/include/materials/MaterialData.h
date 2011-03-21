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

#include "MaterialProperty.h"

class MaterialData
{
public:
  virtual ~MaterialData()
  {
    {
      MaterialProperties::iterator it;
      for (it = _props.begin(); it != _props.end(); ++it)
      {
        if (it->second != NULL)
        {
          delete it->second;
          it->second = NULL;
        }
      }

      for (it = _props_old.begin(); it != _props_old.end(); ++it)
      {
        if (it->second != NULL)
        {
          delete it->second;
          it->second = NULL;
        }
      }
      for (it = _props_older.begin(); it != _props_older.end(); ++it)
      {
        if (it->second != NULL)
        {
          delete it->second;
          it->second = NULL;
        }
      }
    }
  }

  MaterialProperties & props() { return _props; }
  MaterialProperties & propsOld() { return _props_old; }
  MaterialProperties & propsOlder() { return _props_older; }

protected:
  MaterialProperties _props;
  MaterialProperties _props_old;
  MaterialProperties _props_older;

};

#endif /* MATERIALDATA_H_ */
