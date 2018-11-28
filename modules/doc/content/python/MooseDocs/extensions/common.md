# Common Extension

The common extension allows for definition of global shortcuts.

Within the configuration file shortcuts may be defined. For example, the following is a portion
for the configuration file used to generate this page. This defines the "libMesh" and "MOOSE"
website shortcuts, which may be used as traditional markdown shortcuts, as shown in [common-example].

```yaml
globals:
    type: MooseDocs.extensions.common
    shortcuts:
        libMesh: http://libmesh.github.io/
        MOOSE: http://www.mooseframework.org
```

!devel example id=common-example caption=Example use of globally defined shortcut.
[MOOSE] is based on [libMesh]
