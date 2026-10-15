#!/usr/bin/env python3
"""Generate OpenPlugin's actual community contract using the built package hash."""
import argparse
import hashlib
import json
from pathlib import Path
import re

def manifest(package=None,version='0.1.0'):
    result={
        'schema':'openplugin.community/1','id':'microfreeze','name':'Microfreeze',
        'author':'MagicStino','repo':'MagicStino/microfreeze',
        'summary':'Experimental independent synth: 16 oscillator prototypes, four-voice paraphony and a 5x7 modulation matrix. Original sound equivalence and full preset compatibility are unverified.',
        'kind':'instrument','category':'synth','license':'MIT',
        'tags':['synth','cleanroom','experimental','force','armv7'],
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
    p.add_argument('--version',default='0.1.0')
    p.add_argument('--output',type=Path,required=True)
    a=p.parse_args()
    a.output.write_text(json.dumps(manifest(a.package,a.version),indent=2)+'\n')
