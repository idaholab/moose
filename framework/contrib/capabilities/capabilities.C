//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#define PY_SSIZE_T_CLEAN
#include <Python.h>

#include "CapabilityUtils.h"
#include "Moose.h"
#include <string>

static PyObject *
capabilities_check(PyObject *self, PyObject *args)
{
  // function arguments
  const char *requirement;
  PyObject *capdict;
  if (!PyArg_ParseTuple(args, "sO", &requirement, &capdict))
  {
    PyErr_SetString(PyExc_ValueError,
                    "capabilities.check requires two arguments, a requirements string and a "
                    "dictionary with string keys and tuple values.");
    return nullptr;
  }

  // make sure argument 2 is a dictionary
  if (!PyDict_Check(capdict))
  {
    PyErr_SetString(PyExc_ValueError, "The second argument must be a dictionary.");
    return nullptr;
  }

  // map to be populated
  CapabilityUtils::Registry capabilities;

  // get dictionary items
  PyObject *items = PyDict_Items(capdict);
  Py_ssize_t len = PyList_Size(items);
  for (Py_ssize_t i = 0; i < len; ++i)
  {
    PyObject *item = PyList_GetItem(items, i);
    // check if the item is a tuple
    if (!PyTuple_Check(item) || PyTuple_Size(item) != 2)
    {
      PyErr_SetString(PyExc_ValueError,
                      "Items of the capability dictionary myst be tuples (internal error).");
      return nullptr;
    }

    PyObject *key_obj = PyTuple_GetItem(item, 0);
    PyObject * value_doc = PyTuple_GetItem(item, 1);

    // ensure key is a string
    if (!PyUnicode_Check(key_obj))
    {
      PyErr_SetString(PyExc_ValueError, "The dictionary keys must be strings.");
      return nullptr;
    }
    const std::string key(PyUnicode_AsUTF8(key_obj));

    // split value_doc into value and doc string
    if (!PyList_Check(value_doc) || PyList_Size(value_doc) != 2)
    {
      PyErr_SetString(PyExc_ValueError, "The dictionary values must be lists of length two.");
      return nullptr;
    }
    PyObject * value = PyList_GetItem(value_doc, 0);
    PyObject * doc_obj = PyList_GetItem(value_doc, 1);

    // make sure doc is a string
    if (!PyUnicode_Check(doc_obj))
    {
      PyErr_SetString(
          PyExc_ValueError,
          "The second list item in the dictionary values must be strings (documentation).");
      return nullptr;
    }
    const std::string doc = PyUnicode_AsUTF8(doc_obj);

    // determine value type
    if (PyBool_Check(value))
      capabilities[key] = {bool(PyObject_IsTrue(value)), doc};
    else if (PyLong_Check(value))
      capabilities[key] = {static_cast<int>(PyLong_AsLong(value)), doc};
    // else if (PyFloat_Check(value))
    //   capabilities[key] = {PyFloat_AsDouble(value), doc};
    else if (PyUnicode_Check(value))
      capabilities[key] = {std::string(PyUnicode_AsUTF8(value)), doc};
    else
    {
      PyErr_SetString(PyExc_ValueError, "The capability values must be Bool, Number, or Str.");
      return nullptr;
    }
  }

  // throw on errors to properly report the failure back to python
  Moose::_throw_on_error = true;

  try
  {
    // call capabilities C++ code with capabilities map
    auto [status, message, doc] = CapabilityUtils::check(requirement, capabilities);
    return Py_BuildValue("(NNN)",
                         PyLong_FromLong(status),
                         PyUnicode_FromString(message.c_str()),
                         PyUnicode_FromString(doc.c_str()));
  }
  catch (const std::exception & e)
  {
    PyErr_SetString(PyExc_ValueError, e.what());
    return nullptr;
  }
}

static PyMethodDef CapabilitiesMethods[] = {
    {"check",
     capabilities_check,
     METH_VARARGS,
     "Check a requirement against a capabilities dictionary."},
    {nullptr, nullptr, 0, nullptr}};

PyDoc_STRVAR(capabilities_doc, "Interface to the Moose::Capabilities system.");

static struct PyModuleDef capabilitiesmodule = {
    PyModuleDef_HEAD_INIT,
    "capabilities",   /* name of module */
    capabilities_doc, /* module documentation, may be nullptr */
    -1,               /* size of per-interpreter state of the module,
                         or -1 if the module keeps state in global variables. */
    CapabilitiesMethods};

PyMODINIT_FUNC
PyInit_capabilities(void)
{
  auto module = PyModule_Create(&capabilitiesmodule);
  PyModule_AddIntConstant(module, "CERTAIN_FAIL", 0);
  PyModule_AddIntConstant(module, "POSSIBLE_FAIL", 1);
  PyModule_AddIntConstant(module, "UNKNOWN", 2);
  PyModule_AddIntConstant(module, "POSSIBLE_PASS", 3);
  PyModule_AddIntConstant(module, "CERTAIN_PASS", 4);
  PyModule_AddIntConstant(module, "PARSE_FAIL", 5);
  return module;
}
