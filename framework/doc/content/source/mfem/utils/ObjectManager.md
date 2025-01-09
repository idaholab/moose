# ObjectManager

## Summary

`ObjectManager` is a factory class used to create tracked objects in Platypus.

## Overview

`ObjectManager` is used to create and manage tracked objects in Platypus. Iterator methods are
provided to allow access to all of the objects that have been created.

`ObjectManager` objects are used to create and store `mfem::Coefficient`, `mfem::VectorCoefficient`,
and `mfem::MatrixCoefficient` derived objects added to the MFEM problem.

End users should not usually need to interact with the `ObjectManager` directly. Developers wanting
to add new objects tracked by the `ObjectManager` should do so using the `ObjectManager::make`
method.
