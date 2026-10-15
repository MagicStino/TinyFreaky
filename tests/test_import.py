import importlib.util
import json
from pathlib import Path
import unittest

ROOT=Path(__file__).resolve().parents[1]
spec=importlib.util.spec_from_file_location('import_presets',ROOT/'tools/import-presets.py')
module=importlib.util.module_from_spec(spec)
spec.loader.exec_module(module)

def pack(data):
    out=[]
    for i in range(0,len(data),7):
        block=data[i:i+7]
        out.append(sum(((v>>7)&1)<<j for j,v in enumerate(block)))
        out.extend(v&127 for v in block)
    return bytes(out)

def field(name,res,q):
    return bytes([64+len(name)])+name.encode()+b'c'+bytes([res,q&255,q>>8])

class ImportTests(unittest.TestCase):
    def test_unpack_all_msb_positions(self):
        original=bytes(range(256))*2
        self.assertEqual(module.unpack(pack(original)),original)
    def test_named_fields_and_sequence_boundary(self):
        raw=b'#VCO'+field('Type',22,1489)+b'@#Co2'+field('LFO',0,49152)+b'@ '+b'opaque-sequence'
        items,sequence=module.records(raw)
        self.assertEqual([(i['group'],i['name']) for i in items],[('VCO','Type'),('Co2','LFO')])
        self.assertEqual(sequence,b'opaque-sequence')
        self.assertEqual(items[1]['q15'],49152)
    def test_payload_uses_count_and_prefix(self):
        raw=b'#VCO'+field('Type',12,2731)+b'@ '
        packed=pack(raw)
        data=('22 serialization::archive 10 name '+str(len(packed))+' '+' '.join(map(str,packed))).encode()
        self.assertEqual(module.payload(data)[0],raw)
        with self.assertRaises(ValueError): module.payload(data.replace(str(len(packed)).encode(),b'999999',1))
    def test_non_sysex_values_rejected(self):
        with self.assertRaises(ValueError): module.unpack(bytes([0,128]))
    def test_unsupported_original_model_cannot_become_init_patch(self):
        params=json.loads((ROOT/'vst/params.json').read_text())
        for q,expected in [(2731,True),(32767,False)]:
            # R14 final engine is original vocoder, absent from the prototype.
            raw=b'#VCO'+field('Type',14,q)+b'@ '
            packed=pack(raw)
            archive=('22 serialization::archive 10 '+str(len(packed))+' '+' '.join(map(str,packed))).encode()
            result=module.translate(archive,'test',params)
            self.assertEqual(result['engine_loadable'],expected)
            self.assertEqual(result['status'],'partial')

if __name__=='__main__': unittest.main()
