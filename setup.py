try:
    from setuptools import setup
except ImportError:
    from distutils.core import setup

setup(
    name="Visualiser",
    version="0.1-SNAPSHOT",
    description="Spinnaker Visualiser Framework",
    url="https://github.com/SpiNNakerManchester/Visualiser",
    packages=['visualiser_framework'],
    install_requires=['pygtk'],
    scripts=['scripts/virtualenv-gtk2.0-fedora32.sh',
             'scripts/virtualenv-gtk2.0-fedora64.sh',
             'scripts/virtualenv-gtk2.0-ubuntu.sh',
             'scripts/virtualenv-gtk2.0-windows.bat']
)
