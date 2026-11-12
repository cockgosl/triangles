# Triangles

A C++ application for exploring triangle intersections in 3D. Read a scene from
standard input or generate one, then fly through it using mouse and keyboard
controls. Colors distinguish intersecting triangles from the rest of the scene.

## Preview

<p align="center">
  <img src="docs/flythrough.gif"
       alt="Flying through a 3D scene of orange and blue triangles"
       width="800">
</p>

## Features

- Read triangle coordinates from standard input and print the intersecting indices.
- Generate a chosen number of triangles with random positions and orientations.
- Identify triangle intersections using an octree to narrow down candidate pairs.
- Explore a static scene with a freely moving camera.
- View the triangles with lighting and shadows.

## Color guide

| Color | Meaning |
| --- | --- |
| Orange | The triangle does not intersect any other triangle. |
| Blue | The triangle intersects at least one other triangle. |

The color applies to the entire triangle, rather than just the intersection area.

## Building

You need CMake, a C++20 compiler, Git to fetch dependencies, and Python 3 for the
end-to-end tests. Run these commands from the repository root:

```sh
cmake -S . -B build
cmake --build build
```

The application and tests are enabled by default. If an existing build was
configured with `BUILD_APP=OFF`, use a fresh build directory or reconfigure with
`-DBUILD_APP=ON`.

## Using the application

```sh
./build/triangles.out < scene.txt
./build/triangles.out --console < scene.txt
./build/triangles.out --generate
```

The input starts with a triangle count (`0 < N < 1,000,000`), followed by nine
coordinates per triangle (`ax ay az bx by bz cx cy cz`). The default mode prints
the result and opens the scene. With `--console`, the program prints the result
and exits. Each intersecting triangle's index is printed once, in ascending
order, starting from zero, one per line. A successful run with no intersections
produces no output.

In `--generate` mode, enter the number of triangles when prompted. Generator,
octree, and camera settings are currently configured in the source code.

| Input | Action |
| --- | --- |
| Mouse | Look around |
| `W` / `S` | Move forward / backward along the viewing direction |
| `A` / `D` | Move left / right |
| `Esc` | Close the application |

The mouse cursor is captured while the application is running.

## Testing

```sh
(cd build && ctest --output-on-failure)
```

CTest runs the GoogleTest suite and the Python end-to-end tests. The end-to-end
tests use `--console`, so they can run without a display. See the
[manual graphics check](tests/e2e/README.md#manual-graphics-check) to verify the
window, highlighting, and controls.

## limitations

If generating is rejected - config should be adjusted.
