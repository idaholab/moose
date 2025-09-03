"""
A package for building tree structures. This package is designed as a faster alternative to the
anytree package, although it is not a direct replacement.
"""

from .Node import Node as Node
from .search import (
    findall as findall,
    find as find,
    iterate as iterate,
    IterMethod as IterMethod,
)
