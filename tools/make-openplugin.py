#!/usr/bin/env python3
"""Generate OpenPlugin's actual community contract using the built package hash."""
import argparse
import hashlib
import json
from pathlib import Path
import re

def manifest(package=None,version='0.4.0',image=None):
    result={
        'schema':'openplugin.community/1','id':'tinyfreaky','name':'Tinyfreaky',
        'author':'MagicStino','repo':'MagicStino/TinyFreaky',
        'summary':'MagicStino eight-voice synth with 16 engines, output scope, 5x7 matrix, arp/swing and 64-step sequencer. Sound-informed names and stable IDs across 456 slots: 342 playable slots, 114 unavailable/muted. Sequence/sample payloads are not fully supported.',
        'kind':'instrument','category':'synth','license':'MIT',
        'tags':['synth','experimental','force','armv7'],
        'images':[image or f'https://github.com/MagicStino/TinyFreaky/releases/download/v{version}/tinyfreaky-artwork.png'],
        'packages':[]
    }
    if package:
        if not re.fullmatch(r'[0-9]+\.[0-9]+\.[0-9]+(?:-[A-Za-z0-9.-]+)?',version):
            raise ValueError('Invalid release version')
        result['packages']=[{
            'version':version,'arch':'armv7','layout':'portable','devices':['force'],
            'url':f'https://github.com/MagicStino/TinyFreaky/releases/download/v{version}/{package.name}',
            'size':package.stat().st_size,'sha256':hashlib.sha256(package.read_bytes()).hexdigest()
        }]
    return result

if __name__=='__main__':
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--package',type=Path)
    p.add_argument('--version',default='0.4.0')
    p.add_argument('--image',help='Public HTTPS catalog preview URL')
    p.add_argument('--output',type=Path,required=True)
    a=p.parse_args()
    a.output.write_text(json.dumps(manifest(a.package,a.version,a.image),indent=2)+'\n')
