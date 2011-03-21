#ifndef MATERIALDATA_H_
#define MATERIALDATA_H_

#include "MaterialProperty.h"

namespace Moose
{

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

} // namespace

#endif /* MATERIALDATA_H_ */
