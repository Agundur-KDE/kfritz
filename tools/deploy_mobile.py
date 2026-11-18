#!/usr/bin/env python3
"""Prepare the reviewed KFritz extension for the next login. Never restart Plasma."""
from pathlib import Path
import subprocess

ROOT=Path(__file__).resolve().parents[1]
DROPIN=Path.home()/'.config/systemd/user/plasma-plasmashell.service.d/50-kfritz-mobile.conf'

def main():
    subprocess.run(['/usr/bin/python3',str(ROOT/'tools/activate_mobile_on_start.py'),'--dry-run'],check=True)
    subprocess.run(['pkexec','/usr/bin/python3',str(ROOT/'tools/install_staged_atomic.py')],check=True)
    DROPIN.parent.mkdir(parents=True,exist_ok=True)
    # A missing project or activation error must never prevent the desktop starting.
    DROPIN.write_text('[Service]\nExecStartPre=-/usr/bin/python3 '+str(ROOT/'tools/activate_mobile_on_start.py')+'\n')
    subprocess.run(['systemctl','--user','daemon-reload'],check=True)
    print('Prepared for next login; current Plasma session was not restarted.')

if __name__=='__main__':main()
