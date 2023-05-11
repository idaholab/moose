Update Conda:

```bash
mamba activate moose
mamba update --all
```

!alert note title=Always Update MOOSE and the Conda/Mamba packages together
There is a tight dependency between libraries being provided by Conda, and what libraries MOOSE
depends on. Therefore, when you update one you should always update the other.
