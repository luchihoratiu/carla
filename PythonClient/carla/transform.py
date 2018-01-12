# Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB), and the INTEL Visual Computing Lab.
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

import math

from collections import namedtuple

try:
    import numpy
except ImportError:
    raise RuntimeError('cannot import numpy, make sure numpy package is installed.')


Translation = namedtuple('Translation', 'x y z')
Translation.__new__.__defaults__ = (0.0, 0.0, 0.0)

Rotation = namedtuple('Rotation', 'pitch yaw roll')
Rotation.__new__.__defaults__ = (0.0, 0.0, 0.0)

Scale = namedtuple('Scale', 'x y z')
Scale.__new__.__defaults__ = (1.0, 1.0, 1.0)


class Transform(object):
    """A 3D transformation.

    The transformation is applied in the order: scale, rotation, translation.
    """

    @staticmethod
    def from_protobuf_transform(transform):
        """Create a Transform from a Protobuf Transform object."""
        return Transform(translation=transform.location, rotation=transform.rotation)

    def __init__(self, *args, **kwargs):
        if 'matrix' in kwargs:
            self.matrix = kwargs['matrix']
        else:
            self.matrix = numpy.matrix(numpy.identity(4))
            self.set(*args, **kwargs)

    def set(self, translation=Translation(), rotation=Rotation(), scale=Scale()):
        cy = math.cos(numpy.radians(rotation.yaw))
        sy = math.sin(numpy.radians(rotation.yaw))
        cr = math.cos(numpy.radians(rotation.roll))
        sr = math.sin(numpy.radians(rotation.roll))
        cp = math.cos(numpy.radians(rotation.pitch))
        sp = math.sin(numpy.radians(rotation.pitch))
        self.matrix[0, 3] = translation.x
        self.matrix[1, 3] = translation.y
        self.matrix[2, 3] = translation.z
        self.matrix[0, 0] = scale.x * (cp * cy)
        self.matrix[0, 1] = scale.y * (cy * sp * sr - sy * cr)
        self.matrix[0, 2] = -scale.z * (cy * sp * cr + sy * sr)
        self.matrix[1, 0] = scale.x * (sy * cp)
        self.matrix[1, 1] = scale.y * (sy * sp * sr + cy * cr)
        self.matrix[1, 2] = scale.z * (cy * sr - sy * sp * cr)
        self.matrix[2, 0] = scale.x * (sp)
        self.matrix[2, 1] = -scale.y * (cp * sr)
        self.matrix[2, 2] = scale.z * (cp * cr)

    def inverse(self):
        """Return the inverse transform."""
        return Transform(matrix=numpy.linalg.inv(self.matrix))

    def transform_points(self, points):
        """Transform a point-cloud."""
        raise NotImplementedError

    def __mul__(self, other):
        return Transform(matrix=numpy.dot(self.matrix, other.matrix))

    def __str__(self):
        return str(self.matrix)
