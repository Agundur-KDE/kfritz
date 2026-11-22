#!/usr/bin/env python3
"""Wait for the exact package source/build, independently of repository dirtiness."""
import argparse
import json
import re
import subprocess
import time
from urllib.parse import quote, urlencode
import xml.etree.ElementTree as ET


def osc(path):
    return subprocess.run(['osc', 'api', path], check=True, capture_output=True, timeout=60).stdout


def inspect_release(fetch, project, package, repository, arches, tag, commit):
    source = '/source/' + quote(project, safe=':') + '/' + quote(package, safe='')
    service = ET.fromstring(fetch(source + '/_service'))
    revisions = [p.text for s in service.findall('service') if s.get('name') == 'obs_scm'
                 for p in s.findall('param') if p.get('name') == 'revision']
    if revisions != [tag]:
        raise RuntimeError('OBS source service is not pinned to the release tag')
    directory = ET.fromstring(fetch(source + '?expand=1'))
    if directory.find('serviceinfo') is None or directory.find('serviceinfo').get('code') != 'succeeded':
        return None
    revision, source_md5 = directory.get('rev'), directory.get('srcmd5')
    if not revision or not source_md5:
        raise RuntimeError('Missing OBS source revision')
    info_names = [n.get('name') for n in directory.findall('entry') if n.get('name', '').endswith('.obsinfo')]
    if len(info_names) != 1:
        raise RuntimeError('Missing or ambiguous OBS SCM metadata')
    info = fetch(source + '/' + quote(info_names[0], safe=':') + '?' + urlencode({'rev': revision, 'expand': '1'})).decode()
    actual_commit = next((line.split(':', 1)[1].strip() for line in info.splitlines() if line.startswith('commit:')), '')
    if actual_commit != commit:
        return None  # A newly requested source service may still expose the previous source.
    builds = {}
    for arch in arches:
        path = '/build/' + quote(project, safe=':') + '/' + quote(repository, safe='')
        results = ET.fromstring(fetch('/build/' + quote(project, safe=':') + '/_result?' + urlencode({
            'package': package, 'repository': repository, 'arch': arch,
        })))
        statuses = [s for result in results.findall('result') for s in result.findall('status') if s.get('package') == package]
        if len(statuses) != 1:
            return None
        status = statuses[0].get('code')
        if status in {'failed', 'broken', 'unresolvable'}:
            raise RuntimeError('OBS package build failed for ' + arch + ': ' + status)
        if status != 'succeeded':
            return None
        history = ET.fromstring(fetch(path + '/' + quote(arch, safe='') + '/' + quote(package, safe='') + '/_history?limit=1'))
        latest = history.find('entry')
        if latest is None or latest.get('srcmd5') != source_md5 or latest.get('rev') != revision:
            return None
        if latest.get('versrel', '').rsplit('-', 1)[0] != tag.removeprefix('v'):
            raise RuntimeError('OBS build version differs from the release tag')
        builds[arch] = dict(latest.attrib)
    # Catch a source update racing the per-architecture checks.
    current = ET.fromstring(fetch(source + '?expand=1'))
    if current.get('srcmd5') != source_md5 or current.get('rev') != revision:
        return None
    return {'tag': tag, 'commit': commit, 'source_revision': revision, 'source_md5': source_md5, 'builds': builds}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--project', default='home:Agundur')
    parser.add_argument('--package', default='KFritz')
    parser.add_argument('--repository', default='openSUSE_Tumbleweed')
    parser.add_argument('--arches', nargs='+', default=['x86_64', 'i586'])
    parser.add_argument('--tag', required=True)
    parser.add_argument('--commit', required=True)
    parser.add_argument('--timeout', type=int, default=1200)
    parser.add_argument('--interval', type=int, default=20)
    args = parser.parse_args()
    if not re.fullmatch(r'[0-9a-f]{40}', args.commit) or args.timeout < 1 or args.interval < 1:
        parser.error('Expected a full commit SHA and positive wait limits')
    deadline = time.monotonic() + args.timeout
    while True:
        proof = inspect_release(osc, args.project, args.package, args.repository, args.arches, args.tag, args.commit)
        if proof:
            print(json.dumps(proof, indent=2))
            return
        if time.monotonic() >= deadline:
            raise SystemExit('Timed out waiting for the exact OBS release package build')
        print('Waiting for matching package source and successful architecture builds...', flush=True)
        time.sleep(min(args.interval, max(0, deadline - time.monotonic())))


if __name__ == '__main__':
    main()
