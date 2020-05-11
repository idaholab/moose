## MacOS Catalina Caveats id=catalinacaveats

- GCC is not completely functional.

  - [There appears to be an issue/bug with Xcode](https://gcc.gnu.org/bugzilla/show_bug.cgi?id=90835), which prevents many GNU sources from compiling while not using the Xcode IDE. However, there is a work-around. But unfortunately it requires some know-how on your part. The `stdlib.h` header file located at `/usr/include` within the basepath of `xcrun --show-sdk-path`, needs to be patched. The following represents the required change:
  
    ```diff
    diff --git a/usr/include/stdlib.h b/usr/include/stdlib.h
    index 035e6c0..035bb92 100644
    --- a/usr/include/stdlib.h
    +++ b/usr/include/stdlib.h
    @@ -58,8 +58,8 @@
     #ifndef _STDLIB_H_
     #define _STDLIB_H_
     
    -#include <Availability.h>
     #include <sys/cdefs.h>
    +#include <Availability.h>
     
     #include <_types.h>
     #if !defined(_ANSI_SOURCE)
    ```

  - Basically, we want to swap the includes, so that `#include <sys/cdefs.h>` is included before `#include <Availability.h>`. Keep in mind, this change will need to be executed each time Catalina goes through an Xcode update. At the time of this writing, the latest Xcode (11.2) is affected.
