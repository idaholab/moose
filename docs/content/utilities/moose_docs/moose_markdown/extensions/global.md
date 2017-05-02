# MooseDocs Global Extension
The original [markdown] syntax includes the ability to define links using ids:
```
This is [an example][id] of a reference-style link.
```
Then anywhere in the document, typically at the beginning or end, the link must be defined:
```
[id]: http://example.com/
```

The global extension provides a global list of reference links that are added to each markdown page,
to allow for commonly used links to be quickly and uniformly applied. The link definitions are
simply defined in the extension configuration [YAML] file, which within MOOSE points to another
[YAML] file (global.yml), which is shown in \ref{global-yml}

!listing docs/globals.yml id=global-yml caption=List of global markdown reference links.

!extension GlobalExtension
