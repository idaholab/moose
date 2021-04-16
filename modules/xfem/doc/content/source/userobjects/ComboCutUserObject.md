# ComboCutUserObject

!syntax description /UserObjects/ComboCutUserObject

## Overview

The `ComboCutUserObject` combines multiple `GeometricCutUserObject`s to create a composite cut. Currently, all `GeometricCutUserObject`s are assumed to be non-overlapping, i.e. each element can only be cut by one geometric cut.

Each `GeometricCutUserObject` may have defined its own set of `CutSubdomainID`s, and the `ComboCutUserObject` follows the key-value dictionary provided by the user to assign a `CutSubdomainID` to each of the cut subdomains.

## Example Input File Syntax

!syntax parameters /UserObjects/ComboCutUserObject

!syntax inputs /UserObjects/ComboCutUserObject

!syntax children /UserObjects/ComboCutUserObject
