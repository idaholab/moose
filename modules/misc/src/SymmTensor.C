#include "SymmTensor.h"

std::ostream&
operator<<(std::ostream & stream, const SymmTensor & obj)
{
  stream << "SymmTensor:\n"
         << std::setprecision(6)
         << std::setw(13) << obj._xx << "\t" << std::setw(13) << obj._xy << "\t" << std::setw(13) << obj._zx << "\n"
         << "\t\t" << std::setw(13) << obj._yy << "\t" << obj._yz << "\n"
         << "\t\t\t\t" << std::setw(13) << obj._zz << std::endl;
  return stream;
}

template <>
PropertyValue *
MaterialProperty<SymmTensor>::init (int size)
{
  MaterialProperty<SymmTensor> *copy = new MaterialProperty<SymmTensor>;
  copy->_value.resize(size);
  for (unsigned int i(0); i < static_cast<unsigned>(size); ++i)
    (*copy)[i].zero();
  return copy;
}
