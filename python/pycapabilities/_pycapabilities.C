//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#define PY_SSIZE_T_CLEAN
#include <Python.h>

#if PY_VERSION_HEX < 0x030C0000 /* < 3.12 */
#include "structmember.h"
#ifndef Py_T_OBJECT_EX
#define Py_T_OBJECT_EX T_OBJECT_EX
#endif
#ifndef Py_READONLY
#define Py_READONLY READONLY
#endif
#endif

#include "CapabilityException.h"
#include "CapabilityRegistry.h"

#include <memory>

using Moose::Capability;
using Moose::CapabilityException;
using Moose::internal::CapabilityRegistry;

/*******************************************************************************/
/* Python pycapabilities module definition                                     */
/*******************************************************************************/

PyDoc_STRVAR(capabilities_doc, "Interface to the Moose::Capabilities system.");

static struct PyModuleDef pycapabilitiesmodule = {
    PyModuleDef_HEAD_INIT,
    "capabilities",   /* name of module */
    capabilities_doc, /* module documentation, may be nullptr */
    -1,               /* size of per-interpreter state of the module,
                         or -1 if the module keeps state in global variables. */};

/*******************************************************************************/
/* Python pycapabilities.CapabilityException                                   */
/*******************************************************************************/

static PyObject * CapabilityExceptionObject;

/*******************************************************************************/
/* Python capabilities helpers                                                 */
/*******************************************************************************/

void
setPythonError(PyObject * exc, const std::string & message)
{
  PyErr_SetString(exc, message.c_str());
}

PyObject *
returnPythonError(PyObject * exc, const std::string & message)
{
  setPythonError(exc, message);
  return nullptr;
}

/// Add capabilities to a Registry given a python dict representation
static void
addCapabilities(CapabilityRegistry & registry, PyObject * capabilities_dict)
{
  if (!PyDict_Check(capabilities_dict))
    return setPythonError(PyExc_ValueError, "Capabilities item must be a dictionary");

  auto items = PyDict_Items(capabilities_dict);
  const auto len = PyList_Size(items);
  for (Py_ssize_t i = 0; i < len; ++i)
  {
    auto item = PyList_GetItem(items, i);
    auto item_key_obj = PyTuple_GetItem(item, 0);
    auto item_value_obj = PyTuple_GetItem(item, 1);

    // ensure key is a string
    auto name_string = PyUnicode_AsUTF8(item_key_obj);
    if (!name_string)
      return setPythonError(PyExc_TypeError, "The capability dictionary keys must be strings.");
    const std::string name(name_string);

    // error helpers
    const auto error_prefix = [&name]() { return "Capability '" + name + "': "; };
    const auto type_error = [&error_prefix](const auto & message)
    { return setPythonError(PyExc_TypeError, error_prefix() + message); };
    const auto value_error = [&error_prefix](const auto & message)
    { return setPythonError(PyExc_ValueError, error_prefix() + message); };
    const auto key_error = [&error_prefix](const auto & message)
    { return setPythonError(PyExc_KeyError, error_prefix() + message); };

    // check value dict
    if (!PyDict_Check(item_value_obj))
      return type_error("value is not a dictionary");

    // Helper for getting a key from the value dict
    const auto query_key = [&item_value_obj](const auto & key)
    {
      const auto key_obj = PyUnicode_FromString(key);
      return PyDict_Contains(item_value_obj, key_obj) ? PyDict_GetItem(item_value_obj, key_obj)
                                                      : nullptr;
    };

    // 'doc' value
    std::string doc;
    if (auto doc_obj = query_key("doc"))
    {
      if (auto doc_string = PyUnicode_AsUTF8(doc_obj))
        doc = doc_string;
      else
        return type_error("value 'doc' not a string");
    }
    else
      return key_error("missing key 'doc'");

    // 'value' value
    Capability::Value value;
    if (auto value_obj = query_key("value"))
    {
      if (value_obj == Py_True)
        value = true;
      else if (value_obj == Py_False)
        value = false;
      else if (PyLong_Check(value_obj))
        value = static_cast<int>(PyLong_AsLong(value_obj));
      else if (auto value_string = PyUnicode_AsUTF8(value_obj))
      {
        value = std::string(value_string);
      }
      else
        return setPythonError(PyExc_TypeError, error_prefix() + "value 'value' of unexpected type");
    }
    else
      return key_error("missing key 'value'");

    // add capability, checking for errors
    auto & capability = registry.add(name, value, doc);

    // 'explicit' value
    if (auto explicit_compare_obj = query_key("explicit"))
    {
      if (explicit_compare_obj == Py_True)
        capability.setExplicit();
      else if (explicit_compare_obj != Py_False)
        return type_error("'explicit' value not a bool");
    }

    // enumeration entry
    if (const auto enumeration_obj = query_key("enumeration"))
    {
      auto enumeration_iter = PyObject_GetIter(enumeration_obj);
      if (!enumeration_iter)
        return value_error("'enumeration' value not iterable");

      std::set<std::string> enumeration;

      PyObject * enumeration_item_obj;
      while ((enumeration_item_obj = PyIter_Next(enumeration_iter)))
      {
        auto enumeration_string = PyUnicode_AsUTF8(enumeration_item_obj);
        if (!enumeration_string)
        {
          Py_DECREF(enumeration_item_obj);
          Py_DECREF(enumeration_iter);
          return type_error("'enumeration' value is not a string");
        }
        enumeration.insert(enumeration_string);
        Py_DECREF(enumeration_item_obj);
      }
      Py_DECREF(enumeration_iter);
      if (PyErr_Occurred())
        return value_error("failed to iterate through 'enumeration'");

      capability.setEnumeration(std::move(enumeration));
    }
  }
}

/*******************************************************************************/
/* Python pycapabilities.CheckState: IntEnum                                   */
/*******************************************************************************/

static int
CheckState_enum(PyObject * module)
{
  int rc = -1;

  PyObject * enum_mod = nullptr;
  PyObject * IntEnum = nullptr;
  PyObject * name = nullptr;
  PyObject * members = nullptr;
  PyObject * checkstate = nullptr;

  enum_mod = PyImport_ImportModule("enum");
  if (!enum_mod)
    goto done;

  IntEnum = PyObject_GetAttrString(enum_mod, "IntEnum");
  if (!IntEnum)
    goto done;

  name = PyUnicode_FromString("CheckState");
  if (!name)
    goto done;

  members = PyDict_New();
  if (!members)
    goto done;

#define ADD_MEMBER(dict, key, value_long)                                                          \
  do                                                                                               \
  {                                                                                                \
    PyObject * tmp = PyLong_FromLong((value_long));                                                \
    if (!tmp)                                                                                      \
      goto done;                                                                                   \
    if (PyDict_SetItemString((dict), (key), tmp) < 0)                                              \
    {                                                                                              \
      Py_DECREF(tmp);                                                                              \
      goto done;                                                                                   \
    }                                                                                              \
    Py_DECREF(tmp);                                                                                \
  } while (0)

  ADD_MEMBER(members, "CERTAIN_FAIL", CapabilityRegistry::CheckState::CERTAIN_FAIL);
  ADD_MEMBER(members, "POSSIBLE_FAIL", CapabilityRegistry::CheckState::POSSIBLE_FAIL);
  ADD_MEMBER(members, "UNKNOWN", CapabilityRegistry::CheckState::UNKNOWN);
  ADD_MEMBER(members, "POSSIBLE_PASS", CapabilityRegistry::CheckState::POSSIBLE_PASS);
  ADD_MEMBER(members, "CERTAIN_PASS", CapabilityRegistry::CheckState::CERTAIN_PASS);
#undef ADD_MEMBER

  checkstate = PyObject_CallFunctionObjArgs(IntEnum, name, members, nullptr);
  if (!checkstate)
    goto done;

  if (PyModule_AddObject(module, "CheckState", checkstate) < 0)
    goto done;
  checkstate = NULL;

  rc = 0;

done:
  Py_XDECREF(enum_mod);
  Py_XDECREF(IntEnum);
  Py_XDECREF(name);
  Py_XDECREF(members);
  Py_XDECREF(checkstate);
  return rc;
}

/*******************************************************************************/
/* Python pycapabilities.AUGMENTED_CAPABILITY_NAMES: Set[str]                  */
/*******************************************************************************/

static int
AUGMENTED_CAPABILITY_NAMES_set(PyObject * module)
{
  PyObject * names = PySet_New(nullptr);
  if (!names)
    return -1;

  for (const auto & name : CapabilityRegistry::augmented_capability_names)
  {
    PyObject * s = PyUnicode_FromString(name.c_str());
    if (!s)
    {
      Py_DECREF(names);
      return -1;
    }
    if (PySet_Add(names, s) < 0)
    {
      Py_DECREF(s);
      Py_DECREF(names);
      return -1;
    }
    Py_DECREF(s);
  }

  if (PyModule_AddObject(module, "AUGMENTED_CAPABILITY_NAMES", names) < 0)
  {
    Py_DECREF(names);
    return -1;
  }

  return 0;
}

/*******************************************************************************/
/* Python pycapabilities.Capabilities object                                   */
/*******************************************************************************/

/// C++ state stored with a pycapabilities.Capabilities object
struct CapabilitiesState
{
  CapabilityRegistry registry;
};

/// Python state stored with a pycapabilities.Capabilities object
typedef struct
{
  PyObject_HEAD CapabilitiesState * state;
  PyObject * values;
} CapabilitiesObject;

/// Instantiate CapabilitiesState for a pycapabilities.Capabilities object
static PyObject *
Capabilities_new(PyTypeObject * type, PyObject * /* args */, PyObject * /* kwds */)
{
  auto * self = (CapabilitiesObject *)type->tp_alloc(type, 0);
  if (!self)
    return nullptr;

  self->state = nullptr;
  try
  {
    self->state = new CapabilitiesState();
  }
  catch (const std::bad_alloc &)
  {
    Py_DECREF(self);
    PyErr_NoMemory();
    return nullptr;
  }
  return (PyObject *)self;
}

/// pycapabilities.Capabilities.__init__()
static int
Capabilities_init(CapabilitiesObject * self, PyObject * args, PyObject * /* kwds */)
{
  // Parse arguments
  PyObject * capabilities_dict = nullptr;
  if (!PyArg_ParseTuple(args, "O!", &PyDict_Type, &capabilities_dict))
  {
    PyErr_SetString(PyExc_TypeError, "Capabilities expects a single dict argument");
    return -1;
  }

  // Store the provided dict
  Py_INCREF(capabilities_dict);
  self->values = capabilities_dict;

  // Clear registry on new init
  delete self->state;
  self->state = new CapabilitiesState();

  // Add each capability in the dict to the C++ Registry
  try
  {
    addCapabilities(self->state->registry, capabilities_dict);
  }
  catch (const CapabilityException & e)
  {
    PyErr_SetString(CapabilityExceptionObject, e.what());
    return -1;
  }
  catch (const std::exception & e)
  {
    PyErr_SetString(PyExc_RuntimeError, e.what());
    return -1;
  }

  return PyErr_Occurred() ? -1 : 0;
}

/// pycapabilities.Capabilities.check()
static PyObject *
Capabilities_check(CapabilitiesObject * self, PyObject * args, PyObject * kwargs)
{
  // Parse arguments
  const char * requirement = nullptr;
  PyObject * add_capabilities = Py_None;
  PyObject * negate_capabilities = Py_None;
  static const char * kwlist[] = {
      "requirement", "add_capabilities", "negate_capabilities", nullptr};
  if (!PyArg_ParseTupleAndKeywords(args,
                                   kwargs,
                                   "s|OO",
                                   const_cast<char **>(kwlist),
                                   &requirement,
                                   &add_capabilities,
                                   &negate_capabilities))
    return returnPythonError(PyExc_ValueError,
                             "Capabilities.check(requirement: str, add_capabilities: "
                             "Optional[dict] = None, negate_capabilities: Iterable[str]] = None)");

  // Possible copy of the registry for when we need to augment it
  std::unique_ptr<CapabilityRegistry> registry_copy;

  // If adding extra capabilities, copy construct the registry and add
  // the extra capabilities to the copy
  if (add_capabilities != Py_None)
  {
    if (!PyDict_Check(add_capabilities))
      return returnPythonError(PyExc_TypeError, "add_capabilities must be a dict");

    registry_copy = std::make_unique<CapabilityRegistry>();
    *registry_copy = self->state->registry;

    try
    {
      addCapabilities(*registry_copy, add_capabilities);
    }
    catch (const CapabilityException & e)
    {
      PyErr_SetString(CapabilityExceptionObject, e.what());
      return nullptr;
    }
    catch (const std::exception & e)
    {
      PyErr_SetString(PyExc_RuntimeError, e.what());
      return nullptr;
    }
  }

  // If negating capabilities, copy construct (if not already)
  // the registry and negate the capability values
  if (negate_capabilities != Py_None)
  {
    if (!registry_copy)
    {
      registry_copy = std::make_unique<CapabilityRegistry>();
      *registry_copy = self->state->registry;
    }

    auto iter = PyObject_GetIter(negate_capabilities);
    if (!iter)
      returnPythonError(PyExc_TypeError, "'negate_capabilities' not iterable");

    PyObject * obj;
    while ((obj = PyIter_Next(iter)))
    {
      auto name_obj = PyUnicode_AsUTF8(obj);
      if (!name_obj)
      {
        Py_DECREF(obj);
        return returnPythonError(PyExc_TypeError, "'negate_capabilities' entry not a string");
      }
      const std::string name = name_obj;
      Py_DECREF(obj);

      auto capability_ptr = registry_copy->query(name);

      try
      {
        // If the capability exists, negate it (set to false)
        if (capability_ptr)
          capability_ptr->negateValue();
        // If it doesn't exist, just add it
        else
          registry_copy->add(name, false, "Negated capability");
      }
      catch (const CapabilityException & e)
      {
        PyErr_SetString(CapabilityExceptionObject, e.what());
        Py_DECREF(iter);
        return nullptr;
      }
      catch (const std::exception & e)
      {
        PyErr_SetString(PyExc_RuntimeError, e.what());
        Py_DECREF(iter);
        return nullptr;
      }
    }

    Py_DECREF(iter);
    if (PyErr_Occurred())
      return nullptr;
  }

  // Use the copied registry if we're adding and/or negating capabilities,
  // otherwise use the stored registry
  auto & registry = registry_copy ? *registry_copy : self->state->registry;

  // Get a CheckState enum
  PyObject * m = PyState_FindModule(&pycapabilitiesmodule);
  if (!m)
  {
    PyErr_SetString(PyExc_RuntimeError, "module not initialized");
    return nullptr;
  }

  // Run the check
  try
  {
    const auto result = registry.check(requirement);

    PyObject * checkstate = PyObject_GetAttrString(m, "CheckState");
    if (!checkstate)
      return nullptr;

    PyObject * checkstate_obj = PyObject_CallFunction(checkstate, "l", result.state);
    Py_DECREF(checkstate);

    return Py_BuildValue("(NNN)",
                         checkstate_obj,
                         PyUnicode_FromString(result.reason.c_str()),
                         PyUnicode_FromString(result.doc.c_str()));
  }
  catch (const CapabilityException & e)
  {
    PyErr_SetString(CapabilityExceptionObject, e.what());
    return nullptr;
  }
  catch (const std::exception & e)
  {
    PyErr_SetString(PyExc_RuntimeError, e.what());
    return nullptr;
  }
}

/// pycapabilities.Capabilities destruction
static void
Capabilities_dealloc(CapabilitiesObject * self)
{
  Py_XDECREF(self->values);
  delete self->state;
  self->state = nullptr;
  Py_TYPE(self)->tp_free((PyObject *)self);
}

/// pycapabilities.Capabilities definition of methods
static PyMethodDef Capabilities_methods[] = {{"check",
                                              (PyCFunction)Capabilities_check,
                                              METH_VARARGS | METH_KEYWORDS,
                                              "Check a capability expression."},
                                             {nullptr, nullptr, 0, nullptr}};

/// pycapabilities.Capabilities definition of members
static PyMemberDef Capabilities_members[] = {{"values",
                                              Py_T_OBJECT_EX,
                                              offsetof(CapabilitiesObject, values),
                                              Py_READONLY,
                                              "Capabilities dictionary"},
                                             {nullptr}};

/// pycapabilities.Capabilities definition
static PyTypeObject CapabilitiesType = {
    PyVarObject_HEAD_INIT(nullptr, 0).tp_name = "pycapabilities.Capabilities",
    .tp_basicsize = sizeof(CapabilitiesObject),
    .tp_itemsize = 0,
    .tp_dealloc = (destructor)Capabilities_dealloc,
    .tp_flags = Py_TPFLAGS_DEFAULT,
    .tp_doc = "Python representation of a MOOSE capability registry.",
    .tp_methods = Capabilities_methods,
    .tp_members = Capabilities_members,
    .tp_init = (initproc)Capabilities_init,
    .tp_new = Capabilities_new};

/*******************************************************************************/
/* Python pycapabilities module init                                           */
/*******************************************************************************/

PyMODINIT_FUNC
PyInit__pycapabilities(void)
{
  if (PyType_Ready(&CapabilitiesType) < 0)
    return nullptr;

  auto module = PyModule_Create(&pycapabilitiesmodule);

  // CapabilityException
  CapabilityExceptionObject =
      PyErr_NewException("pycapabilities.CapabilityException", PyExc_Exception, nullptr);
  if (!CapabilityExceptionObject)
  {
    Py_DECREF(module);
    return nullptr;
  }
  Py_INCREF(CapabilityExceptionObject);
  PyModule_AddObject(
      module, "CapabilityException", CapabilityExceptionObject); // steals a ref to CustomError

  // CheckState: IntEnum
  if (CheckState_enum(module) < 0)
  {
    Py_DECREF(module);
    return nullptr;
  }

  // AUGMENTED_CAPABILITY_NAMES: Set[str]
  if (AUGMENTED_CAPABILITY_NAMES_set(module) < 0)
  {
    Py_DECREF(module);
    return nullptr;
  }

  // Capabilities object
  Py_INCREF(&CapabilitiesType);
  PyModule_AddObject(module, "Capabilities", (PyObject *)&CapabilitiesType);

  return module;
}
