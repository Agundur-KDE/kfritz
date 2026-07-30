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
    read -rp "FritzBox IP [192.168.178.1]: " HOST
    HOST="${HOST:-192.168.178.1}"
fi
if [[ -z "$LOGIN" ]]; then
    read -rp "TR-064 username: " LOGIN
fi
if [[ -z "$PASSWORD" ]]; then
    read -rsp "TR-064 password: " PASSWORD
    echo
fi

echo "KFritz Setup Check against $HOST"
echo

# 1. TR-064 port reachable
if timeout 3 bash -c "echo > /dev/tcp/$HOST/$PORT" 2>/dev/null; then
    ok "TR-064 port $PORT reachable"
else
    fail "TR-064 port $PORT NOT reachable"
    hint "Home Network → Network → Network Settings → enable 'Access for Applications' (TR-064)"
    exit 1
fi

# 2. CallMonitor port reachable
if timeout 3 bash -c "echo > /dev/tcp/$HOST/1012" 2>/dev/null; then
    ok "CallMonitor port 1012 reachable"
else
    fail "CallMonitor port 1012 NOT reachable"
    hint "Dial #96*5* from a phone connected to the FritzBox (enables it permanently, survives reboots)"
fi

if [[ -z "$LOGIN" || -z "$PASSWORD" ]]; then
    echo
    echo "No username/password given — phonebook/call-list check skipped."
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
    ok "Phonebook access works ($COUNT phonebook(s): $IDS)"
elif echo "$RESPONSE" | grep -qi "unauthorized"; then
    fail "TR-064 login failed — check username/password"
elif echo "$RESPONSE" | grep -q "<errorCode>"; then
    ERR=$(echo "$RESPONSE" | grep -oP '(?<=<errorDescription>).*?(?=</errorDescription>)')
    CODE=$(echo "$RESPONSE" | grep -oP '(?<=<errorCode>).*?(?=</errorCode>)')
    fail "FritzBox reports an error: ${ERR:-unknown} (errorCode ${CODE:-?})"
    hint "System → FRITZ!Box Users → user needs 'Voice messages, faxes, FRITZ!App Fon and call list'"
    hint "If the permission is already correct: is this box running as a Mesh repeater/IP client? Telephony services are often inactive there."
else
    fail "Unexpected/no response (connection problem?)"
fi

# 4. Call list access — same permission checkbox, but a distinct AVM feature,
# worth checking separately (missed-call catch-up depends on it).
RESPONSE=$(soap_call "GetCallList")
if echo "$RESPONSE" | grep -q "<NewCallListURL>http"; then
    ok "Call list access works (needed for missed-calls catch-up)"
elif echo "$RESPONSE" | grep -q "<NewCallListURL></NewCallListURL>\|<NewCallListURL/>"; then
    fail "Call list returns an empty URL — feature might be disabled on the box"
else
    fail "Call list not retrievable"
    hint "Same permission as phonebook needed, see above"
fi

echo
echo "Done."
