#!/usr/bin/env python3
"""Black-box tests for triangles.out; run: python3 test_e2e.py /path/to/triangles.out."""
import argparse
import itertools
import os
import pathlib
import random
import subprocess
import sys
import tempfile
import time
import unittest

BINARY = None
OBSERVE_SECONDS = 1.0
ERROR_TIMEOUT = 5.0


def tri(a, b, c):
    return (a, b, c)


def move(t, x=0, y=0, z=0):
    return tuple((a+x, b+y, c+z) for a, b, c in t)


def scene_input(scene):
    return str(len(scene)) + '\n' + ''.join(' '.join(str(n) for p in t for n in p) + '\n' for t in scene)


BASE = tri((0, 0, 0), (4, 0, 0), (0, 4, 0))
CASES = [
    ('single', [BASE], []),
    ('separate_coplanar', [BASE, move(BASE, 10)], []),
    ('overlapping_boxes_but_disjoint', [BASE, tri((4, 4, 0), (4, 1, 0), (1, 4, 0))], []),
    ('parallel_planes', [BASE, move(BASE, z=1)], []),
    ('identical', [BASE, BASE], [0, 1]),
    ('contained', [BASE, tri((1, 1, 0), (2, 1, 0), (1, 2, 0))], [0, 1]),
    ('coplanar_crossing', [BASE, tri((2, -1, 0), (5, 2, 0), (2, 5, 0))], [0, 1]),
    ('vertex_touch', [BASE, tri((4, 0, 0), (5, 0, 0), (4, -1, 0))], [0, 1]),
    ('edge_touch', [BASE, tri((0, 0, 0), (4, 0, 0), (0, -4, 0))], [0, 1]),
    ('partial_edge_overlap', [BASE, tri((1, 0, 0), (3, 0, 0), (2, -2, 0))], [0, 1]),
    ('noncoplanar_cross', [BASE, tri((1, 1, -1), (1, 1, 1), (3, 1, 0))], [0, 1]),
    ('noncoplanar_vertex_touch', [BASE, tri((1, 1, 0), (1, 1, 2), (2, 1, 2))], [0, 1]),
    ('noncoplanar_disjoint', [BASE, tri((3, 3, -1), (3, 3, 1), (4, 4, 1))], []),
    ('mixed_indices', [move(BASE, 40), BASE, move(BASE, 20), tri((1, 1, 0), (2, 1, 0), (1, 2, 0)), move(BASE, 60)], [1, 3]),
    ('several_partners_once_each', [BASE, BASE, BASE, move(BASE, 20)], [0, 1, 2]),
]


def many_scene():
    # Isolated cells, spaced 20 apart. Every seventh cell has a contained triangle.
    # No geometry from different cells can intersect; exact expected indices follow
    # directly from construction, independently of the production intersection code.
    scene, expected = [], []
    for i in range(100):
        x, y, z = (i % 10) * 20 - 90, (i // 10) * 20 - 90, ((i % 3) - 1) * 20
        scene.append(move(BASE, x, y, z))
        if i % 7 == 0:
            expected.extend((len(scene)-1, len(scene)))
            scene.append(move(tri((1, 1, 0), (2, 1, 0), (1, 2, 0)), x, y, z))
    return scene, expected


class E2E(unittest.TestCase):
    def assert_scene(self, scene, expected, args=()):
        # Files avoid pipe backpressure; stdin is closed after the complete input.
        # There is no end-of-output marker, so the bounded observation interval is
        # a practical smoke test, not proof that an empty answer is final.
        with tempfile.TemporaryFile() as inp, tempfile.TemporaryFile() as out, tempfile.TemporaryFile() as err:
            inp.write(scene_input(scene).encode('ascii'))
            inp.seek(0)
            p = subprocess.Popen([BINARY, *args], stdin=inp, stdout=out, stderr=err)
            try:
                deadline = time.monotonic() + OBSERVE_SECONDS
                while time.monotonic() < deadline and p.poll() is None:
                    time.sleep(min(0.02, max(0, deadline-time.monotonic())))
                status = p.poll()
            finally:
                if p.poll() is None:
                    p.terminate()
                    try:
                        p.wait(timeout=2)
                    except subprocess.TimeoutExpired:
                        p.kill()
                        p.wait(timeout=2)
            out.seek(0)
            err.seek(0)
            actual = out.read().decode('utf-8', errors='replace')
            errors = err.read().decode('utf-8', errors='replace')
            self.assertIsNone(status, f'process exited before observation ended: {status}; stdout={actual!r}; stderr={errors!r}')
            self.assertEqual(actual, ''.join(f'{i}\n' for i in expected), f'stdout mismatch; stderr={errors!r}')

    def test_named_scenes(self):
        for name, scene, expected in CASES:
            with self.subTest(name=name):
                self.assert_scene(scene, expected)

    def test_explicit_input_option(self):
        self.assert_scene([BASE, BASE], [0, 1], args=('--input',))

    def test_vertex_and_triangle_order(self):
        other = tri((1, 1, -1), (1, 1, 1), (3, 1, 0))
        # Six winding/order variants, and both input orders.
        for vertices in itertools.permutations(BASE):
            for swap in (False, True):
                with self.subTest(vertices=vertices, swap=swap):
                    pair = [vertices, other]
                    self.assert_scene(pair[::-1] if swap else pair, [0, 1])

    def test_large_octotree_scene_and_seeded_permutation(self):
        scene, expected = many_scene()
        self.assert_scene(scene, expected)
        order = list(range(len(scene)))
        random.Random(5489).shuffle(order)
        expected_set = set(expected)
        self.assert_scene([scene[i] for i in order], [j for j, i in enumerate(order) if i in expected_set])

    def test_invalid_inputs(self):
        invalid = [
            ('missing_count', '', 'Expected an integer'),
            ('zero_count', '0\n', 'Expected an integer'),
            ('negative_count', '-1\n', 'Expected an integer'),
            ('fractional_count', '1.5\n', 'Expected an integer'),
            ('too_many', '1000000\n', 'Expected an integer'),
            ('count_overflow', '184467440737095516160\n', 'Expected an integer'),
            ('missing_coordinate', '1\n0 0 0 1 0 0 0 1\n', 'Triangle 1:'),
            ('nonnumeric_coordinate', '1\n0 0 0 1 word 0 0 1 0\n', 'Triangle 1:'),
            ('nonfinite', '1\nnan 0 0 1 0 0 0 1 0\n', 'Triangle 1:'),
            ('duplicate_vertex', '1\n0 0 0 0 0 0 0 1 0\n', 'Triangle 1:'),
            ('collinear', '1\n0 0 0 1 1 1 2 2 2\n', 'Triangle 1:'),
            ('second_triangle_incomplete', '2\n0 0 0 1 0 0 0 1 0\n0 0\n', 'Triangle 2:'),
        ]
        for name, data, message in invalid:
            with self.subTest(name=name):
                try:
                    p = subprocess.run([BINARY, '--input'], input=data, text=True, capture_output=True, timeout=ERROR_TIMEOUT)
                except subprocess.TimeoutExpired:
                    self.fail(f'{name}: did not exit within {ERROR_TIMEOUT}s')
                self.assertEqual(p.returncode, 1, f'{name}: stdout={p.stdout!r}, stderr={p.stderr!r}')
                self.assertEqual(p.stdout, '')
                self.assertIn(message, p.stderr)

    def test_invalid_argument(self):
        p = subprocess.run([BINARY, '--unknown'], input='', text=True, capture_output=True, timeout=ERROR_TIMEOUT)
        self.assertEqual(p.returncode, 1)
        self.assertEqual(p.stdout, '')
        self.assertIn('Usage:', p.stderr)


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('binary', type=pathlib.Path)
    parser.add_argument('--observe', type=float, default=1.0, help='seconds to observe each valid process (default: 1)')
    args, unittest_args = parser.parse_known_args()
    if not args.binary.is_file() or not os.access(args.binary, os.X_OK):
        parser.error('binary must be an executable file')
    if args.observe <= 0:
        parser.error('--observe must be positive')
    BINARY = str(args.binary.resolve())
    OBSERVE_SECONDS = args.observe
    unittest.main(argv=[sys.argv[0], *unittest_args], verbosity=2)
