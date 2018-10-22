# Panoptic Extension

The panoptic extension allows for definition of global shortcuts, the name "panoptic" is simply
a fancy word for global. The word global is a keyword in python so it was avoided as an extension
name.

Within the configuration file shortcuts may be defined. For example, the following is a portion
for the configuration file used to generate this page. This defines the "libMesh" and "MOOSE"
website shortcuts, which may be used as traditional markdown shortcuts, as shown in [panoptic-example].

```yaml
globals:
    type: MooseDocs.extensions.panoptic
    shortcuts:
        libMesh: http://libmesh.github.io/
        MOOSE: http://www.mooseframework.org
```

!devel example id=panoptic-example caption=Example use of globally defined shortcut.
[MOOSE] is based on [libMesh]
