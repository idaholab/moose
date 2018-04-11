# Line Search System

This system is meant for creating custom line searches. In general the line
searches associated with the underlying non-linear solver library should be
sufficient. Custom line searches should inherit from `LineSearch` (a pure
virtual) and implement some line-searching capability. The only currently implemented line search is a
[`ContactLineSearch`](/ContactLineSearch.md) which helps resolve the non-smooth
function behavior introduced by contact.
