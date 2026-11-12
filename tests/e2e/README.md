# End-to-end tests for triangles

Run the console tests with the built executable:

```sh
python3 tests/e2e/test_e2e.py /absolute/path/to/triangles.out
```

The tests pass scenes through standard input to `--console`, wait for the process to exit, and check the exit status and exact output. An empty output with exit status 0 means no triangles intersect. The suite includes coplanar and 3D cases, touching, vertex permutations, a larger octree scene, and invalid input.

## Manual graphics check

Run the executable without options and enter a scene with two intersecting triangles and one separate triangle:

```text
3
0 0 0  4 0 0  0 4 0
1 1 0  2 1 0  1 2 0
10 0 0  14 0 0  10 4 0
```

Check that the window opens, the first two triangles are highlighted in blue, the third is orange, and the window closes with Esc.
