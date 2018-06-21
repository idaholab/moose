## Change Ownership

We are done building libraries, so lets chown up the target directory appropriately

```bash
sudo chown -R root:root $PACKAGES_DIR
```

This is more of a formality step, so any potential user of your newly built compiler stack does not
see everything owned by a non-root user.
