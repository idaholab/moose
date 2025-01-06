#ifdef MFEM_ENABLED

#pragma once

#include <memory>
#include <type_traits>
#include <exception>

namespace PlatypusUtils
{

template <class Td, class Ts>
std::shared_ptr<Td>
dynamic_const_cast(std::shared_ptr<Ts> source_pointer)
{
  if constexpr (!std::is_const_v<Td>)
  {
    auto source_remove_const =
        std::const_pointer_cast<typename std::remove_const<Ts>::type>(source_pointer);
    auto destination_pointer = std::dynamic_pointer_cast<Td>(source_remove_const);
    if (!destination_pointer)
      throw std::bad_cast();
    return destination_pointer;
  }
  else
  {
    auto destination_pointer = std::dynamic_pointer_cast<Td>(source_pointer);
    if (!destination_pointer)
      throw std::bad_cast();
    return destination_pointer;
  }
}

} // namespace PlatypusUtils

#endif
