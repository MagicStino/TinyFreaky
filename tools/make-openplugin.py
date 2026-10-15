#!/usr/bin/env python3
"""Generate OpenPlugin's actual community contract using the built package hash."""
import argparse
import hashlib
import json
from pathlib import Path
import re

def manifest(package=None,version='0.3.0-preview.1',image=None):
    result={
        'schema':'openplugin.community/1','id':'tinyfreaky','name':'Tinyfreaky',
        'author':'MagicStino','repo':'MagicStino/microfreeze',
        'summary':'Synth with a readable 5x7 matrix, arp, swing and editable 64-step sequencer. Eleven oscillator paths use original Mutable Instruments cores. 448 factory entries: 334 translated, 114 require unavailable engines. Original sound equivalence is unverified.',
        'kind':'instrument','category':'synth','license':'MIT',
        'tags':['synth','experimental','force','armv7'],
        'images':[image or f'https://github.com/MagicStino/microfreeze/releases/download/v{version}/tinyfreaky-artwork.png'],
        'packages':[]
    }
    if package:
        if not re.fullmatch(r'[0-9]+\.[0-9]+\.[0-9]+(?:-[A-Za-z0-9.-]+)?',version):
            raise ValueError('Invalid release version')
        result['packages']=[{
            'version':version,'arch':'armv7','layout':'portable','devices':['force'],
            'url':f'https://github.com/MagicStino/microfreeze/releases/download/v{version}/{package.name}',
            'size':package.stat().st_size,'sha256':hashlib.sha256(package.read_bytes()).hexdigest()
        }]
    return result

if __name__=='__main__':
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--package',type=Path)
    p.add_argument('--version',default='0.3.0-preview.1')
    p.add_argument('--image',help='Public HTTPS catalog preview URL')
    p.add_argument('--output',type=Path,required=True)
    a=p.parse_args()
    a.output.write_text(json.dumps(manifest(a.package,a.version,a.image),indent=2)+'\n')
