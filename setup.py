try:
    from setuptools import setup
except ImportError:
    from distutils.core import setup

setup(
    name="Visualiser",
    version="0.1-SNAPSHOT",
    description="Spinnaker Visualiser Framework",
    url="https://github.com/SpiNNakerManchester/Visualiser",
    packages=['visualiser_framework']
)
