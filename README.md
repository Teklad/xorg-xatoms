=== XAtoms ===
XAtoms is a tool for printing and/or searching for Xorg server atoms.  It can
match based on partial or full names of Atoms.

Examples:
```bash
# To match all atoms containing the keyword NET_WM in the range 300-400
xatoms --range=300-400 --name=NET_WM --partial

# For an exact match
xatoms --name=WM_NAME

# Format it nice and pretty
xatoms --format="%s\t\t%d\n"
```


