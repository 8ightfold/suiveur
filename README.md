# suiveur
This library builds on ideas from 
[Angi Yaghi's](https://github.com/ange-yaghi) memory tracker.
It provides several utilities to track the use of raw pointers, 
printing "rust style" errors when used incorrectly.
This allows you to use unsafe allocation 
without worrying about incorrect usage.

## Example usage
```cpp
int* iptr = REGISTER_ALLOC(new int { 77 });
float* fptr = REGISTER_ALLOC(new float { 6.9f });
double* dptr = new double { 5.5 };  // untracked

/* double free */
SAFE_DELETE(iptr);
SAFE_DELETE(iptr);
/* free of punned pointer */
SAFE_DELETE((char*)fptr);
/* free of untracked pointer */
SAFE_DELETE(dptr);
```

## Docs
### Functions
Suiveur defines several macros that can be used for tracking:
```cpp
T* REGISTER_ALLOC(T*);
T* REGISTER_DELETE(T*);
T* SAFE_DELETE(T*);

void RESET_REGISTRY();
void PASS_REGISTRY();
```

``REGISTER_ALLOC`` and ``REGISTER_DELETE`` will add/remove pointers from the tracking list.
Using ``REGISTER_DELETE`` will not error on invalid usage.

On the other hand, ``SAFE_DELETE`` will. It will warn you when you attempt
to delete an untracked pointer, when you free a pointer multiple times, or when
you free a pointer that has been punned.

``RESET_REGISTRY`` will completely wipe the list of tracked pointers,
printing any errors that have occured. If any pointers have not been freed at
the point of calling, they will be considered unfreed.

``PASS_REGISTRY`` will just wipe the
list of deleted pointers, meaning it will not be able to detect things such 
as double frees for previously tracked pointers. It will not warn you about
pointers that are still unfreed.

### Cmake
Suiveur also provides 2 cmake settings: ``enable_tracking`` and ``disable_ansi``.
The former will turn on tracking, the functions are noops otherwise. 
The latter will turn off colored printing, as certain consoles do not support
ansi escape codes. Both are ``OFF`` by default.

## Notes
Because this library was originally made to track AST nodes (single objects),
the tracking of ``new[]`` is currently unsupported. Attempting to use it
*will* result in UB. Support may be added in the future, but it would probably
be better for you to just use a ``std::vector`` or a ``std::unique_ptr``.

Big thanks to [Rald](https://github.com/smartel99) for coming up with the name :)