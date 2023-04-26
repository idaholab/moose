Update Conda:

```bash
mamba activate moose
mamba update --all
```

!alert note title=Always Update MOOSE and Conda together
There is a tight dependency between libraries being provided by Conda, and what libraries MOOSE
depends on. Therefor, when you update one you should always update the other.
