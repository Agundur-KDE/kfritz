import unittest
from urllib.parse import parse_qs, urlsplit

from wait_obs_release import inspect_release

COMMIT = 'a' * 40


class ObsReleaseGateTest(unittest.TestCase):
    def fetcher(self, *, history_md5='current', commit=COMMIT, status='succeeded'):
        def fetch(path):
            if path.endswith('/_service'):
                return b'<services><service name="obs_scm"><param name="revision">v0.5.0</param></service></services>'
            if '.obsinfo?' in path:
                return ('commit: ' + commit + '\n').encode()
            if '?expand=1' in path:
                return b'<directory rev="40" srcmd5="current"><serviceinfo code="succeeded"/><entry name="_service:obs_scm:kfritz.obsinfo"/></directory>'
            if '/_result?' in path:
                arch = parse_qs(urlsplit(path).query)['arch'][0]
                return ('<resultlist><result repository="openSUSE_Tumbleweed" arch="' + arch + '" code="building" dirty="true"><status package="KFritz" code="' + status + '"/></result></resultlist>').encode()
            if '/_history?' in path:
                return ('<buildhistory><entry rev="40" srcmd5="' + history_md5 + '" versrel="0.5.0-2"/></buildhistory>').encode()
            raise AssertionError(path)
        return fetch

    def inspect(self, fetch):
        return inspect_release(fetch, 'home:Agundur', 'KFritz', 'openSUSE_Tumbleweed', ['x86_64', 'i586'], 'v0.5.0', COMMIT)

    def test_unrelated_repository_build_does_not_block_matching_package(self):
        self.assertEqual(COMMIT, self.inspect(self.fetcher())['commit'])

    def test_previous_package_build_is_rejected_despite_succeeded_status(self):
        self.assertIsNone(self.inspect(self.fetcher(history_md5='previous')))

    def test_old_git_commit_is_not_ready(self):
        self.assertIsNone(self.inspect(self.fetcher(commit='b' * 40)))

    def test_package_failure_aborts(self):
        with self.assertRaisesRegex(RuntimeError, 'failed'):
            self.inspect(self.fetcher(status='failed'))


if __name__ == '__main__':
    unittest.main()
