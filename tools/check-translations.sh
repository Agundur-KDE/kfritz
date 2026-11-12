#!/usr/bin/env bash
set -euo pipefail

project_dir="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)"
for catalog in "$project_dir"/translate/*/*.po; do
    echo "Checking ${catalog#"$project_dir/"}"
    msgfmt --check --check-format --statistics -o /dev/null "$catalog"
    # Reject missing, untranslated or fuzzy messages in any shipped language.
    msgcmp "$catalog" "$project_dir/translate/kfritz.pot"
done
