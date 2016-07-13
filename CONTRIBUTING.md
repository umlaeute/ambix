Contributing to libambix development
====================================

Contribution should be done via
- Merge Requests / Pull Requests
OR
- via git patchsets (`git format-patch`)

# GIT

## Canonical Repositories

MAIN repository
- https://git.iem.at/ambisonics/libambix

secondary repository
- https://github.com/iem-projects/ambix

Please *do not use* these legacy repositories:
- ~~https://github.com/umlaeute/ambix~~
- ~~https://git.code.sf.net/p/iem/ambix~~ (SourceForge)

## Commits

- make small, atomic commits
- commit often
- be sure that the description matches the actual changeset
- do not mix unrelated changes into a single commit (*"fixed bug BAR, added
  feature FOO"* is **bad**)
- never mix changes to different components
  (e.g. changes to `libambix/` should go in different commits as changes to `utils/`
  and changes to `samples/pd/`)
  - even if this temporarily breaks compilation! (e.g. if you are changing the
    API of the libambix library, you have to update any code that uses this API;
    in this case, first commit the changes to libambix, and then commit the
    required changes in the utilities. the latter should be a single commit that
    only adapts the code to the new API)
	- make small, atomic commits

# Coding Style

- libambix is written in `ANSI-C` (aka `C89`)

- function names use underscores to separate names (e.g. `ambix_open`)
- *public* functions
  - each *public* function **MUST** be declared in `libambix/ambix/ambi.h`
  - each *public* function **MUST** be declared with the `AMBIX_API` decorator
  - each *public* function **MUST** start with the `ambix_` prefix (to avoid nameclashes with other libraries)
- *non-public* functions
  - *non-public* functions that are only used within a single source file **MUST** be declared `static`
  - *non-public* functions that are used within the entire libambix library **MUST** be prefixed with `_ambix`
  - *non-public* functions that are used within the entire libambix library **SHOULD** be declared in `libambix/src/private.h` file

- type names use underscores to separate names (e.g. `ambix_matrix_t`)
- type names have a `_t` suffix (and an `ambix_` prefix if they are non-trivial)
- *public* types
  - complex types (aka `struct`s) should always be opaque (the actual `struct` is kept as an implementation detail)
  - if needed, use (public) accessor functions (`get`/`set`) for the elements of a complex type

-

## Code Formatting

TODO (follow mine :-))
- no whitespace at the end-of-line
- no empty line at the end-of-file


# Code Testing

## Unit Tests

See `libambix/tests/` for example tests.

All public functionality (and as far as possible all private functionality)
should be tested via unit tests.
The testing "framework" in use is provided by autotools.

When testing private interfaces (non-public functions), the unit tests
need to be compiled in debug-mode (using the `--enable-debug` configure option)
and the tests should be protected by an `if DEBUG / endif` clause
`libambix/tests/Makefile.am`.

## Checking for memory leaks
The unit tests can be instrumented to be run through valgrind:

    cd libambix/tests
    make check-valgrind

The above will run the tests through the standard `memcheck` tool.
If you want to use another valgrind tool, You can specify that as well:

    cd libambix/tests
    make check-valgrind-tool VALGRIND_TOOL=memcheck

## Continuous Integration
The library is automatically built and tested via
[Travis-CI](https://travis-ci.org/iem-projects/ambix) whenever new code is
committed.

If you do not have write access to the main repository of libambix, you can
Trigger a CI build for your contribution simply by creating a *Pull Request*
against the [iem-projects/ambix](https://github.com/iem-projects/ambix)
repository on GitHub.
