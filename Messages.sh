#!/usr/bin/env bash
# Extract translatable strings from both the configuration and C++ overlay.
mapfile -t sources < <(find package/contents package/plugin -type f \( -name '*.qml' -o -name '*.cpp' \))
"${XGETTEXT:-xgettext}" "${sources[@]}" -o translate/kfritz.pot \
    --from-code=UTF-8 \
    --language=C++ \
    --keyword=i18n --keyword=i18nc:1c,2 --keyword=i18np:1,2 --keyword=i18ncp:1c,2,3 \
    --keyword=i18nd:2 \
    --package-name=kfritz
