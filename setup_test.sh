#!/usr/bin/env bash
# Checks the FritzBox-side prerequisites KFritz needs, before you go looking
# for a bug in the widget itself: CallMonitor port, TR-064 port, phonebook
# access, call-list access. No dependency beyond curl + bash.

set -uo pipefail

GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[0;33m'
NC='\033[0m'

ok()   { echo -e "  ${GREEN}✓${NC} $1"; }
fail() { echo -e "  ${RED}✗${NC} $1"; }
hint() { echo -e "    ${YELLOW}→${NC} $1"; }

HOST="${1:-}"
LOGIN="${2:-}"
PASSWORD="${3:-}"
PORT="${4:-49000}"

if [[ -z "$HOST" ]]; then
    read -rp "FritzBox-IP [192.168.178.1]: " HOST
    HOST="${HOST:-192.168.178.1}"
fi
if [[ -z "$LOGIN" ]]; then
    read -rp "TR-064-Benutzername: " LOGIN
fi
if [[ -z "$PASSWORD" ]]; then
    read -rsp "TR-064-Passwort: " PASSWORD
    echo
fi

echo "KFritz Setup-Check gegen $HOST"
echo

# 1. TR-064 port reachable
if timeout 3 bash -c "echo > /dev/tcp/$HOST/$PORT" 2>/dev/null; then
    ok "TR-064-Port $PORT erreichbar"
else
    fail "TR-064-Port $PORT NICHT erreichbar"
    hint "Heimnetz → Netzwerk → Netzwerkeinstellungen → Zugriff für Anwendungen (TR-064) aktivieren"
    exit 1
fi

# 2. CallMonitor port reachable
if timeout 3 bash -c "echo > /dev/tcp/$HOST/1012" 2>/dev/null; then
    ok "CallMonitor-Port 1012 erreichbar"
else
    fail "CallMonitor-Port 1012 NICHT erreichbar"
    hint "An einem an der FritzBox angeschlossenen Telefon wählen: #96*5* (aktiviert dauerhaft, auch nach Reboot)"
fi

if [[ -z "$LOGIN" || -z "$PASSWORD" ]]; then
    echo
    echo "Kein Benutzername/Passwort angegeben — Telefonbuch-/Anrufliste-Check übersprungen."
    exit 0
fi

SOAP_ENVELOPE_TEMPLATE='<?xml version="1.0" encoding="utf-8"?>
<s:Envelope xmlns:s="http://schemas.xmlsoap.org/soap/envelope/" s:encodingStyle="http://schemas.xmlsoap.org/soap/encoding/">
<s:Body><u:%s xmlns:u="urn:dslforum-org:service:X_AVM-DE_OnTel:1"/></s:Body></s:Envelope>'

soap_call() {
    local action="$1"
    printf -v body "$SOAP_ENVELOPE_TEMPLATE" "$action"
    curl -s --max-time 5 --digest -u "$LOGIN:$PASSWORD" \
        -H 'Content-Type: text/xml; charset="utf-8"' \
        -H "SOAPACTION: \"urn:dslforum-org:service:X_AVM-DE_OnTel:1#$action\"" \
        -d "$body" \
        "http://$HOST:$PORT/upnp/control/x_contact"
}

# 3. Phonebook access
RESPONSE=$(soap_call "GetPhonebookList")
if echo "$RESPONSE" | grep -q "<NewPhonebookList>"; then
    IDS=$(echo "$RESPONSE" | grep -oP '(?<=<NewPhonebookList>).*?(?=</NewPhonebookList>)')
    COUNT=$(echo "$IDS" | tr ',' '\n' | grep -c .)
    ok "Telefonbuch-Zugriff funktioniert ($COUNT Telefonbuch/-bücher: $IDS)"
elif echo "$RESPONSE" | grep -qi "unauthorized"; then
    fail "TR-064-Login fehlgeschlagen — Benutzername/Passwort prüfen"
elif echo "$RESPONSE" | grep -q "<errorCode>"; then
    ERR=$(echo "$RESPONSE" | grep -oP '(?<=<errorDescription>).*?(?=</errorDescription>)')
    fail "FritzBox meldet einen Fehler: ${ERR:-unbekannt}"
    hint "System → FRITZ!Box-Benutzer → Benutzer braucht 'Sprachnachrichten, Faxnachrichten, FRITZ!App Fon und Anrufliste'"
else
    fail "Unerwartete/keine Antwort (Verbindungsproblem?)"
fi

# 4. Call list access — same permission checkbox, but a distinct AVM feature,
# worth checking separately (missed-call catch-up depends on it).
RESPONSE=$(soap_call "GetCallList")
if echo "$RESPONSE" | grep -q "<NewCallListURL>http"; then
    ok "Anrufliste-Zugriff funktioniert (nötig für Missed-Calls-Abgleich)"
elif echo "$RESPONSE" | grep -q "<NewCallListURL></NewCallListURL>\|<NewCallListURL/>"; then
    fail "Anrufliste liefert eine leere URL — Feature evtl. auf der Box deaktiviert"
else
    fail "Anrufliste nicht abrufbar"
    hint "Selbe Berechtigung wie Telefonbuch nötig, siehe oben"
fi

echo
echo "Fertig."
