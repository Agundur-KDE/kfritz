#!/usr/bin/env python3
"""Administrator-only installation of the reviewed KFritz stage, without restarting Plasma.
Replace file inodes atomically so an already loaded plugin keeps its old mapping.
"""
from datetime import datetime
from pathlib import Path
import os
import re
import shutil
import stat
import tempfile

ROOT=Path(__file__).resolve().parents[1]
STAGE=ROOT/'.local/stage'

def allowed(path):
    value=str(path)
    return (value.startswith('/usr/lib64/qt6/qml/de/agundur/kfritz/') or
            value.startswith('/usr/share/plasma/plasmoids/de.agundur.kfritz/') or
            value in ['/usr/share/knotifications6/kfritz.notifyrc','/usr/share/metainfo/de.agundur.kfritz.appdata.xml'] or
            bool(re.fullmatch('/usr/share/locale/[a-z_@-]+/LC_MESSAGES/plasma_applet_de.agundur.kfritz.mo',value)))

def main():
    if os.geteuid()!=0:raise RuntimeError('Administrator authentication required')
    paths=[Path(p) for p in (ROOT/'build/install_manifest_kfritz.txt').read_text().splitlines()]
    for target in paths:
        source=STAGE/target.relative_to('/')
        if not allowed(target) or not source.is_file() or source.is_symlink() or not source.resolve().is_relative_to(STAGE.resolve()):raise RuntimeError('Unexpected staged KFritz file')
    backup=ROOT/'.local'/('system-backup-'+datetime.now().strftime('%Y%m%d-%H%M%S'))
    backup.mkdir(mode=0o700)
    for target in paths:
        source=STAGE/target.relative_to('/')
        if target.exists():
            old=backup/target.relative_to('/');old.parent.mkdir(parents=True,exist_ok=True);shutil.copy2(target,old,follow_symlinks=False)
        target.parent.mkdir(mode=0o755,parents=True,exist_ok=True)
        descriptor,name=tempfile.mkstemp(prefix='.kfritz-install-',dir=target.parent)
        try:
            with os.fdopen(descriptor,'wb') as output,source.open('rb') as input_file:
                shutil.copyfileobj(input_file,output);output.flush();os.fsync(output.fileno())
                os.fchmod(output.fileno(),stat.S_IMODE(source.stat().st_mode)&0o777)
            os.replace(name,target)
        finally:
            if os.path.exists(name):os.unlink(name)
    print('KFritz files installed atomically. No service was stopped or restarted.')

if __name__=='__main__':main()
