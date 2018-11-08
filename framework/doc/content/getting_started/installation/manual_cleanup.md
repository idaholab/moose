## Cleanup

Whith everything finished, it is now safe to remove the temporary directory containing our source:

```bash
if [ -d "$STACK_SRC" ]; then rm -rf "$STACK_SRC"; fi
```
