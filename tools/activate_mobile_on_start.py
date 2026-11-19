#!/usr/bin/env python3
"""One-time configuration immediately before a future Plasma start; never restart it."""
from pathlib import Path
import os,re,shutil,signal,subprocess,tempfile
import sys
from verify_deployment import verify

ROOT=Path(__file__).resolve().parents[1]
CONFIG=Path.home()/'.config/plasma-org.kde.plasma.desktop-appletsrc'
DONE=ROOT/'.local/mobile-activation.done'

def patch(content,key):
    sections=re.split(r'(?m)^(\[[^\n]+\])\n',content)
    groups=[sections[i] for i in range(1,len(sections),2) if re.search(r'(?m)^plugin=de\.agundur\.kfritz$',sections[i+1])]
    if len(groups)!=1:raise RuntimeError('Expected one existing KFritz applet; configuration untouched')
    header=groups[0]+'[Configuration][MobileCalls]'
    values={'MobileEnabled':'true','MobilePort':'10123','MobileKey':key}
    for i in range(1,len(sections),2):
        if sections[i]==header:
            lines=[line for line in sections[i+1].splitlines() if line.split('=',1)[0] not in values]
            while lines and not lines[-1]:lines.pop()
            sections[i+1]='\n'.join(lines+[name+'='+value for name,value in values.items()])+'\n\n'
            return ''.join(part+('\n' if index%2 else '') for index,part in enumerate(sections))
    return content.rstrip()+'\n\n'+header+'\n'+''.join(name+'='+value+'\n' for name,value in values.items())

def main():
    # An activation marker proves configuration only, not installed binaries.
    # RPM updates can overwrite the development receiver between logins.
    # Deployment dry-run checks configuration before repairing installed files.
    if '--dry-run' not in sys.argv:
        verify()
    if DONE.exists():return
    key=(ROOT/'.local/mobile-key').read_text().strip()
    if not re.fullmatch('[a-f0-9]{64}',key):raise RuntimeError('Private pairing key unavailable')
    before=CONFIG.read_text();after=patch(before,key)
    if '--dry-run' in __import__('sys').argv:
        print('Ready: existing KFritz applet can be paired on its next Plasma start.');return
    shutil.copy2(CONFIG,ROOT/'.local/desktop-appletsrc.before-activation.private')
    handle,temporary=tempfile.mkstemp(prefix='.kfritz-mobile-',dir=CONFIG.parent)
    try:
        os.fchmod(handle,0o600)
        with os.fdopen(handle,'w') as file:file.write(after)
        os.replace(temporary,CONFIG)
    finally:
        if os.path.exists(temporary):os.unlink(temporary)
    # Only stop our separate development probe if it survived until this start.
    result=subprocess.run(['pgrep','-f',r'^KFritz/build/bin/kfritz_mobile_probe --key-file /home/alec/projects/KFritz/\.local/mobile-key --port 10123$'],capture_output=True)
    for value in result.stdout.decode().split():
        try:os.kill(int(value),signal.SIGTERM)
        except ProcessLookupError:pass
    DONE.write_text('Mobile call receiver configured for the existing applet.\n')
    print('Existing KFritz applet paired; normal Plasma startup continues.')

if __name__=='__main__':main()
