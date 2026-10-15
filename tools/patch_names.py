# SPDX-License-Identifier: MIT
"""Approved, sound-informed names. Slot IDs stay stable across imports."""
import json
from pathlib import Path
_NAMES=tuple(json.loads((Path(__file__).resolve().parents[1]/'assets/factory/names.json').read_text())['names'])
NATIVE_NAMES=_NAMES[:8]
def patch_name(index,values):
 if not 0<=index<448:raise ValueError('Bank supports 448 stable slots')
 return _NAMES[index+8]
