# Black-box tests for triangles

Run on a desktop where the executable can create its real graphics window:

```sh
python3 test_e2e.py /absolute/path/to/triangles.out
```

The executable is started with standard input in its default mode; no headless option is used. `--observe 2` changes the default one-second observation interval. Additional unittest options can follow the binary path, for example `-k invalid`.

The expected results are specified from the geometry of the input, not calculated by the application. Cases include coplanar and 3D intersections, contact at vertices and edges, containment, separation, multiplicity, order and winding changes, and a reproducibly shuffled scene of 115 triangles. Errors of input and argument handling use `subprocess.run` with a timeout and verify exit code 1 and diagnostics. Valid cases compare the exact stdout, including order and newline characters. The process must remain alive during the observation, and is then terminated with a bounded terminate/kill sequence. stderr is captured and included in failures but is not required to be empty: OS and graphics libraries may emit their own diagnostics.

**Limits:** stdout is flushed before window creation, and has no end marker. A process that is alive with matching output is not proof that window creation or rendering succeeded; in particular, an empty answer cannot be distinguished from a process stuck before output. A passing run does not prove that a window appeared, or that its contents or colors are correct. The runner terminates the process and does not check normal window closure or its exit code. Observation duration is a practical bound, not a completion signal. For a complete GUI check, observe the window manually on a working desktop and close it separately; this suite does not automate that check.

