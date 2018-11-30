## Cleanup

Whith everything finished, it is now safe to remove the temporary directory containing the source tree:

```bash
if [ -d "$STACK_SRC" ]; then rm -rf "$STACK_SRC"; fi
```
