#!/usr/bin/env python3
"""Send a label-only legacy TEST to a local receiver. Never print its private key."""
import argparse,base64,hashlib,hmac,socket,uuid
from pathlib import Path
p=argparse.ArgumentParser();p.add_argument('--key-file',required=True);p.add_argument('--host',default='127.0.0.1');p.add_argument('--port',type=int,default=10123);a=p.parse_args()
key=bytes.fromhex(Path(a.key_file).read_text().strip())
with socket.create_connection((a.host,a.port),3) as connection:
 connection.settimeout(4)
 with connection.makefile('rb') as stream:
  nonce=stream.readline(1025).decode().strip()
  payload='\t'.join(['v1','desktop-probe','TEST',str(uuid.uuid4()),base64.b64encode(b'ScreenRing connection test').decode()])
  signature=hmac.new(key,(nonce+'\n'+payload).encode(),hashlib.sha256).hexdigest()
  connection.sendall((payload+'\t'+signature+'\n').encode())
  if stream.readline(20)!=b'OK\n':raise SystemExit('No acknowledgement')
print('Receiver acknowledged TEST; no caller data sent.')
