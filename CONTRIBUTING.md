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

## Commits

- make small, atomic commits
- commit often
- be sure that the description matches the actual changeset
- do not mix unrelated changes into a single commit ("fixed bug BAR, added
  feature FOO" is *bad*)
- never mix changes to different components
  (e.g. changes to libambix/ should go in different commits as changes to utils/
  and changes to samples/pd)
  - even if this temporarily breaks compilation! (e.g. if you are changing the
    API of the libambix library, you have to update any code that uses this API;
    in this case, first commit the changes to libambix, and then commit the
    required changes in the utilities. the latter should be a single commit that
    only adapts the code to the new API)
- make small, atomic commits

# Coding Style

TODO (follow mine :-))
