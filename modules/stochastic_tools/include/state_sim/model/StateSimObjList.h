/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef STATESIMOBJLIST_H
#define STATESIMOBJLIST_H

#include <string>
#include <iterator>

/**
 * Root for StateSim objects.  Unordered maps for Name or ID object lookup and tracking of deleted items.
 * All StateSim object types should have a derived list class of their type.
 */
template <class T>
class StateSimObjList
{
public:
  /**
   * Main constructor to for a statesim object list.
   */
  StateSimObjList();
  T & add(T * to_add);
  void clear();
  void deleteAll();
  void remove(const int item_id);
  bool contains(const T & to_find);
  int size()
  {
    return _item_list.size();
  };
  T * operator[](int id);
  T * operator[](int id) const;
  T * operator[](std::string id);
  T * operator[](std::string id) const;

  /**
   * setProcessed - set the processed flag in all items in the list.
   * @param value - the value to set the flag
   */
  void setProcessed(bool value);

  /**
   * allProcessed - check if all the items have their processed flag set to true.
   * @return if all the items have their processed flag set to true
   */
  virtual std::string allProcessed();

  bool loaded;

  typename std::unordered_map<int, std::unique_ptr<T>>::iterator begin() { return _item_list.begin(); };
  typename std::unordered_map<int, std::unique_ptr<T>>::iterator end()  { return _item_list.end(); };

protected:
  std::unordered_map<int, std::unique_ptr<T>> _item_list;
  std::unordered_map<std::string, int> _name_to_id;
  std::unordered_map<int, std::unique_ptr<T>> _deleted;
};

template <class T>
StateSimObjList<T>::StateSimObjList()
  : loaded(false)
{
}

template <class T>
T &
StateSimObjList<T>::add(T * to_add)
{
  auto id = to_add->id();
  std::string name = to_add->name();

  typename std::unordered_map<int, std::unique_ptr<T>>::const_iterator got = _item_list.find(to_add->id());
  if (got == _item_list.end())
  {
    //put it in the id and object map
    _item_list.insert(std::make_pair<int, std::unique_ptr<T>>(to_add->id(), nullptr));
    _item_list[id].reset(to_add);

    //put the name and id in the name lookup
    _name_to_id.insert(std::make_pair(name, id));
    mooseAssert(_name_to_id.size() == _item_list.size(), "Item with the same name but different ID already exists '" + name + "'.");
  }
  else
  {
    mooseAssert(false, "Item " + name + " can't be added, a second item named '" + got->second->name() + "' has the same id - " + std::to_string(id));
  }

  return *to_add;
}


template <class T>
void
StateSimObjList<T>::clear()
{
  _item_list.clear();
  _name_to_id.clear();
  _deleted.clear();
}

template <class T>
bool
StateSimObjList<T>::contains(const T & to_find)
{
  typename std::unordered_map<int, std::unique_ptr<T>>::const_iterator got = _item_list.find(to_find.id());
  return (got != _item_list.end());
}

template <class T>
void
StateSimObjList<T>::deleteAll()
{
  //typename std::unordered_map<int, std::unique_ptr<T>>::const_iterator
  //for (const auto & iterator = _item_list.begin(); iterator != _item_list.end(); iterator++)
  for (const auto & pair : _item_list)
  {
    T * item = pair.second;
    _deleted.insert(std::make_pair(item->id(), item));
  }

  _item_list.clear();
  _name_to_id.clear();
}

template <class T>
void
StateSimObjList<T>::remove(const int item_id)
{
  //std::unordered_map<int, std::unique_ptr<T>>::const_iterator
  const auto & got = _item_list.find(item_id);
  if (got != _item_list.end())
  {
    T * item = got->second;
    _deleted.insert(std::make_pair(item->id(), item));

    _item_list.erase(got);
    _name_to_id.erase(item->name());
  }
}

template <class T>
void
StateSimObjList<T>::setProcessed(const bool val)
{
  //std::unordered_map<int, std::unique_ptr<T>>::const_iterator
  //for (const auto & iterator = _item_list.begin(); iterator != _item_list.end(); iterator++)
  for (const auto & pair : _item_list)
  {
    pair.second->setProcessed(val);
  }
}

template <class T>
std::string
StateSimObjList<T>::allProcessed()
{
  std::string ret_not_done = "";

  //typename std::unordered_map<int, std::unique_ptr<T>>::const_iterator
  //for (const auto & iterator = _item_list.begin(); iterator != _item_list.end(); iterator++)
  for (const auto & pair : _item_list)
  {
    T & item = *(pair.second);
    if (!item.processed())
    {
      ret_not_done += item.name() + " Action was not processed." + '\n';
    }
  }

  return ret_not_done;
}

template <class T>
T * StateSimObjList<T>::operator[](int id)
{
  //std::unordered_map<int, std::unique_ptr<T>>::const_iterator
  const auto & got = _item_list.find(id);
  if (got != _item_list.end())
  {
    T & item = *(got->second);
    return &item;
  }
  else
    return nullptr;
}

template <class T>
T * StateSimObjList<T>::operator[](int id) const
{
  //std::unordered_map<int, std::unique_ptr<T>>::const_iterator
  const auto & got = _item_list.find(id);
  if (got != _item_list.end())
  {
    T & item = *(got->second);
    return &item;
  }
  else
    return nullptr;
}

template <class T>
T * StateSimObjList<T>::operator[](std::string name)
{
  //std::unordered_map<std::string, int>::const_iterator
  const auto & got_name = _name_to_id.find(name);
  if (got_name != _name_to_id.end())
  {
    //typename std::unordered_map<int, std::unique_ptr<T>>::const_iterator
    const auto & got = _item_list.find(got_name->second);
    if (got != _item_list.end())
    {
      T & item = *(got->second);
      return &item;
    }
  }

  return nullptr;
}

template <class T>
T * StateSimObjList<T>::operator[](std::string name) const
{
  //typename std::unordered_map<std::string, int>::const_iterator
  const auto & got_name = _name_to_id.find(name);
  if (got_name != _name_to_id.end())
  {
    //typename std::unordered_map<int, std::unique_ptr<T>>::const_iterator
    const auto & got = _item_list.find(got_name->second);
    if (got != _item_list.end())
    {
      T & item = *(got->second);
      return &item;
    }
  }

  return nullptr;
}

#endif
