# Downstream patches for the vendored editor cores

These patches apply to the nested `3dparty/SweetEditor` and `3dparty/SweetLine`
checkouts. A recursive clone initializes both cores at their recorded gitlink
revisions through `.gitmodules`; this also lets `huxerui_use_library(URL, COMMIT)`
acquire the library normally. Source archives and non-recursive clones can still
use the top-level SweetEditor FetchContent fallback, which applies
`sweeteditor-oslog-gcc.patch` automatically. Developers using nested checkouts
must apply both patches themselves:

```sh
git -C 3dparty/SweetEditor apply ../patches/sweeteditor-oslog-gcc.patch
git -C 3dparty/SweetLine apply ../patches/sweetline-locale-include.patch
```

| Patch | Reason |
| --- | --- |
| `sweeteditor-oslog-gcc.patch` | `logging.hpp` routes Apple logging through `<os/log.h>`, which hard-errors under GCC; the patch restricts that path to clang so macOS builds with GCC work. |
| `sweetline-locale-include.patch` | `util.cpp` used `std::wstring_convert` and `std::copy` through transitive includes that libc++ 23 pruned; spell out `<locale>` and `<algorithm>`. Only needed when building the preview example with a nested SweetLine checkout. |
