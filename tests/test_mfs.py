import importlib.util,unittest
from pathlib import Path
p=Path(__file__).resolve().parents[1]/'tools/decode-mfs.py';s=importlib.util.spec_from_file_location('mfs',p);m=importlib.util.module_from_spec(s);s.loader.exec_module(m)
def archive(payload,flag=0):return ('22 serialization::archive 10 0 4 8 DEVBUILD 5 Piano 1 0 0 18 000000000000000000 '+str(flag)+' 0 1 '+str(len(payload))+' '+' '.join(map(str,payload))+'\n').encode()
class MfsTests(unittest.TestCase):
 def test_exact_signed_pcm(self):
  info,pcm=m.decode(archive([0,-128,-1,127]));self.assertEqual(pcm,b'\x00\x80\xff\x7f');self.assertEqual(info['frames'],2);self.assertEqual(info['sample_rate'],32000)
 def test_empty(self):self.assertTrue(m.decode(archive([],1))[0]['empty'])
 def test_malformed(self):
  for data in [archive([1]),archive([128,0]),archive([0,0])+b' extra',archive([0,0])[:-3],archive([0,0],1)]:
   with self.assertRaises(ValueError):m.decode(data)
if __name__=='__main__':unittest.main()
