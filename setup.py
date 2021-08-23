# Copyright (c) 2017-2019 The University of Manchester
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

import os
from collections import defaultdict
try:
    from setuptools import setup
except ImportError:
    from distutils.core import setup

install_requires = ['sPyNNaker == 1!6.0.1']
__version__ = "1!6.0.1"

setup(
    name="sPyNNaker C++ Visualisers Library",
    version=__version__,
    url="https://github.com/SpiNNakerManchester/Visualiser",
    classifiers=[
        "Development Status :: 5 - Production/Stable",

        "Environment :: Console",

        "Intended Audience :: Developers",
        "Intended Audience :: Science/Research",

        "License :: OSI Approved :: GNU General Public License v3 (GPLv3)",

        "Natural Language :: English",

        "Operating System :: POSIX :: Linux",
        "Operating System :: Microsoft :: Windows",
        "Operating System :: MacOS",

        "Programming Language :: C",
        "Programming Language :: Python :: 3",
        "Programming Language :: Python :: 3.6",
        "Programming Language :: Python :: 3.7",
        "Programming Language :: Python :: 3.8",
        "Programming Language :: Python :: 3.9",

        "Topic :: Scientific/Engineering",
        "Topic :: Scientific/Engineering :: Neuroscience",
    ],
    packages=["visualiser_example_binaries"],
    maintainer="SpiNNakerTeam",
    maintainer_email="spinnakerusers@googlegroups.com"
)
