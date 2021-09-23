//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once
/**
 * These are reworked from https://stackoverflow.com/a/11003103
 *
 * New GitHub Repo here: https://github.com/friedmud/unique_ptr_cast
 */

#include <memory>

template <typename T_DEST, typename T_SRC, typename T_DELETER>
std::unique_ptr<T_DEST, T_DELETER>
dynamic_pointer_cast(std::unique_ptr<T_SRC, T_DELETER> & src)
{
  if (!src)
    return std::unique_ptr<T_DEST, T_DELETER>(nullptr);

  T_DEST * dest_ptr = dynamic_cast<T_DEST *>(src.get());
  if (!dest_ptr)
    return std::unique_ptr<T_DEST, T_DELETER>(nullptr);

  std::unique_ptr<T_DEST, T_DELETER> dest_temp(dest_ptr, std::move(src.get_deleter()));

  src.release();

  return dest_temp;
}

template <typename T_DEST, typename T_SRC, typename T_DELETER>
std::unique_ptr<T_DEST, T_DELETER>
dynamic_pointer_cast(std::unique_ptr<T_SRC, T_DELETER> && src)
{
  if (!src)
    return std::unique_ptr<T_DEST, T_DELETER>(nullptr);

  T_DEST * dest_ptr = dynamic_cast<T_DEST *>(src.get());
  if (!dest_ptr)
    return std::unique_ptr<T_DEST, T_DELETER>(nullptr);

  std::unique_ptr<T_DEST, T_DELETER> dest_temp(dest_ptr, std::move(src.get_deleter()));

  src.release();

  return dest_temp;
}

template <typename T_DEST, typename T_SRC>
std::unique_ptr<T_DEST>
dynamic_pointer_cast(std::unique_ptr<T_SRC> & src)
{
  if (!src)
    return std::unique_ptr<T_DEST>(nullptr);

  T_DEST * dest_ptr = dynamic_cast<T_DEST *>(src.get());
  if (!dest_ptr)
    return std::unique_ptr<T_DEST>(nullptr);

  std::unique_ptr<T_DEST> dest_temp(dest_ptr);

  src.release();

  return dest_temp;
}

template <typename T_DEST, typename T_SRC>
std::unique_ptr<T_DEST>
dynamic_pointer_cast(std::unique_ptr<T_SRC> && src)
{
  if (!src)
    return std::unique_ptr<T_DEST>(nullptr);

  T_DEST * dest_ptr = dynamic_cast<T_DEST *>(src.get());
  if (!dest_ptr)
    return std::unique_ptr<T_DEST>(nullptr);

  std::unique_ptr<T_DEST> dest_temp(dest_ptr);

  src.release();

  return dest_temp;
}
