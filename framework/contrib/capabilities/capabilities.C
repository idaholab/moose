#define PY_SSIZE_T_CLEAN
#include <Python.h>

#include "CapabilityUtils.h"
#include <map>
#include <string>
#include <variant>

static PyObject *
capabilities_check(PyObject *self, PyObject *args)
{
  // function arguments
  const char *requirement;
  PyObject *capdict;
  if (!PyArg_ParseTuple(args, "sO", &requirement, &capdict))
    return NULL;

  // make sure argument 2 is a dictionary
  if (!PyDict_Check(capdict))
  {
    // TODO: set exception
    return NULL;
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
      // TODO: set exception
      return NULL;
    }

    PyObject *key_obj = PyTuple_GetItem(item, 0);
    PyObject * value_doc = PyTuple_GetItem(item, 1);

    // ensure key is a string
    if (!PyUnicode_Check(key_obj))
    {
      // TODO: set exception
      return NULL;
    }
    const std::string key(PyUnicode_AsUTF8(key_obj));

    // split value_doc into value and doc string
    if (!PyTuple_Check(value_doc) || PyTuple_Size(item) != 2)
    {
      // TODO: set exception
      return NULL;
    }
    PyObject * value = PyTuple_GetItem(item, 0);
    PyObject * doc_obj = PyTuple_GetItem(item, 1);

    // make sure doc is a string
    if (!PyUnicode_Check(doc_obj))
    {
      // TODO: set exception
      return NULL;
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
      // TODO: set exception
      return NULL;
    }
  }

  // call capabilities C++ code with capabilities map
  auto [status, message] = CapabilityUtils::check(requirement, capabilities);
  return Py_BuildValue("(NN)", PyLong_FromLong(status), PyUnicode_FromString(message.c_str()));
}

static PyMethodDef CapabilitiesMethods[] = {
    {"check", capabilities_check, METH_VARARGS,
     "Check a requirement against a capabilities dictionary."},
    {NULL, NULL, 0, NULL}};

PyDoc_STRVAR(capabilities_doc, "Interface to the Moose::Capabilities system.");

static struct PyModuleDef capabilitiesmodule = {
    PyModuleDef_HEAD_INIT,
    "capabilities",   /* name of module */
    capabilities_doc, /* module documentation, may be NULL */
    -1,               /* size of per-interpreter state of the module,
                         or -1 if the module keeps state in global variables. */
    CapabilitiesMethods};

PyMODINIT_FUNC
PyInit_capabilities(void)
{
  auto module = PyModule_Create(&capabilitiesmodule);
  PyModule_AddIntConstant(module, "CERTAIN_FAIL", 0);
  PyModule_AddIntConstant(module, "POSSIBLE_FAIL", 1);
  PyModule_AddIntConstant(module, "POSSIBLE_PASS", 2);
  PyModule_AddIntConstant(module, "CERTAIN_PASS", 3);
  return module;
}
