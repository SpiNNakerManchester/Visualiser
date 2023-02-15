# Copyright (c) 2017 The University of Manchester
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

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

        "License :: OSI Approved :: Apache License 2.0",

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
