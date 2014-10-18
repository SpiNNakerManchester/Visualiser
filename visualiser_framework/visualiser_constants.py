"""constants used by the visualiser_framework"""
from enum import Enum

VISUALISER_MODES = Enum(
    value="VISUALISER_MODES",
    names=[("RASTER", 0),
           ("TOPOLOGICAL", 1)])