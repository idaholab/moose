/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "SymmTensor.h"

std::ostream &
operator<<(std::ostream & stream, const SymmTensor & obj)
{
  stream << "SymmTensor:\n"
         << std::setprecision(6) << std::setw(13) << obj._xx << "\t" << std::setw(13) << obj._xy
         << "\t" << std::setw(13) << obj._zx << "\n"
         << "\t\t" << std::setw(13) << obj._yy << "\t" << std::setw(13) << obj._yz << "\n"
         << "\t\t\t\t" << std::setw(13) << obj._zz << std::endl;
  return stream;
}

template <>
PropertyValue *
MaterialProperty<SymmTensor>::init(int size)
{
  MaterialProperty<SymmTensor> * copy = new MaterialProperty<SymmTensor>;
  copy->_value.resize(size);
  for (unsigned int i(0); i < static_cast<unsigned>(size); ++i)
    (*copy)[i].zero();
  return copy;
}

template <>
void
dataStore(std::ostream & stream, const SymmTensor & v, void * /*context*/)
{
  Real r;
  r = v.xx();
  stream.write((const char *)&r, sizeof(r));
  r = v.yy();
  stream.write((const char *)&r, sizeof(r));
  r = v.zz();
  stream.write((const char *)&r, sizeof(r));
  r = v.xy();
  stream.write((const char *)&r, sizeof(r));
  r = v.yz();
  stream.write((const char *)&r, sizeof(r));
  r = v.zx();
  stream.write((const char *)&r, sizeof(r));
}

template <>
void
dataLoad(std::istream & stream, SymmTensor & v, void * /*context*/)
{
  Real r = 0.;
  stream.read((char *)&r, sizeof(r));
  v.xx(r);
  stream.read((char *)&r, sizeof(r));
  v.yy(r);
  stream.read((char *)&r, sizeof(r));
  v.zz(r);
  stream.read((char *)&r, sizeof(r));
  v.xy(r);
  stream.read((char *)&r, sizeof(r));
  v.yz(r);
  stream.read((char *)&r, sizeof(r));
  v.zx(r);
}
