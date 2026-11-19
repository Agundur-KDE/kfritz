#!/usr/bin/env python3
"""Check installed KFritz against the reviewed stage without exposing configuration."""
from pathlib import Path
import hashlib

ROOT = Path(__file__).resolve().parents[1]

def verify():
    paths = [Path(value) for value in
             (ROOT / 'build/install_manifest_kfritz.txt').read_text().splitlines()]
    if not paths:
        raise RuntimeError('KFritz installation manifest is empty')
    mismatches = []
    for target in paths:
        staged = ROOT / '.local/stage' / target.relative_to('/')
        if (not staged.is_file() or not target.is_file() or
                hashlib.sha256(staged.read_bytes()).digest() !=
                hashlib.sha256(target.read_bytes()).digest()):
            mismatches.append(str(target))
    if mismatches:
        raise RuntimeError('Installed KFritz differs from the reviewed stage; '
                           'a package update may have replaced it:\n' + '\n'.join(mismatches))
    return len(paths)

if __name__ == '__main__':
    try:
        print(f'All {verify()} installed KFritz files match the reviewed stage.')
    except (OSError, RuntimeError) as error:
        raise SystemExit(str(error))
