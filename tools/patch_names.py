# SPDX-License-Identifier: MIT
"""Stable TinyFreaky names; identity follows slot, never a source-bank label."""
NATIVE_NAMES=('First Light','Silver Drift','Prism Keys','Copper String','Orbit Stack','Dust Bloom','Furnace Bass','Neon Throat')
ADJECTIVES=('Velvet','Neon','Copper','Silver','Lunar','Amber','Cobalt','Ivory','Prism','Ember','Polar','Violet','Solar','Jade','Scarlet','Opal')
OBJECTS=('Orbit','Lantern','Circuit','Comet','Ribbon','Harbor','Prism','Reactor','Garden','Signal','Atlas','Mirage','Petal','Current','Meteor','Tangle','Horizon','Quarry','Capsule','Beacon','Vector','Ripple','Lattice','Shadow','Arcade','Tundra','Spark','Echo')
def patch_name(index,values):
 if not 0<=index<448:raise ValueError('Bank supports 448 stable slots')
 model=int(values[3]);attack=float(values[10]);sustain=float(values[12]);octave=float(values[30])
 role='Pulse' if float(values[70])>.5 else 'Keys'
 if attack>.3:role='Pad'
 elif sustain<.15:role='Pluck'
 elif octave<0:role='Bass'
 elif model in (10,12):role='Texture'
 elif model in (4,11):role='Mallet'
 elif model==9:role='Chord'
 return f'{ADJECTIVES[index%16]} {OBJECTS[index//16]} {role}'
