/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "DiagTensor.h"

std::ostream &
operator<<(std::ostream & stream, const DiagTensor & obj)
{
  stream << "DiagTensor:\n"
         << std::setprecision(3) << std::setw(13) << obj._xx << "\n"
         << "\t\t" << std::setw(13) << obj._yy << "\n"
         << "\t\t\t\t" << std::setw(13) << obj._zz << std::endl;
  return stream;
}

template <>
PropertyValue *
MaterialProperty<DiagTensor>::init(int size)
{
  MaterialProperty<DiagTensor> * copy = new MaterialProperty<DiagTensor>;
  copy->_value.resize(size);
  for (unsigned int i(0); i < static_cast<unsigned>(size); ++i)
    (*copy)[i].zero();
  return copy;
}
