# ScreenRing local call protocol

Receiver default: TCP 10123, opt-in, IPv4 LAN. Each receiver has its own random
32-byte pairing key, represented by 64 lowercase hexadecimal characters.
Never log keys, caller numbers, contact names or decrypted packets.

1. Receiver sends a fresh random 32-byte challenge as 64 lowercase hex characters
   followed by LF. It accepts one packet for that challenge, then closes.
2. Sender sends one UTF-8, tab-separated packet followed by LF (maximum 4096 bytes).
3. Receiver validates and applies the event, then replies with the authenticated
   acknowledgement below. Invalid or capacity-rejected messages are closed without
   acknowledgement. END for an already-ended call succeeds idempotently. An unknown
   CONNECT is not acknowledged. Clients retry independently per receiver.

## Authenticated acknowledgement (ScreenRing 0.6.0-alpha.2)

The response is `ACK<TAB>hex-hmac<LF>`. Its HMAC-SHA256 uses the decoded pairing
key and authenticates the following exact UTF-8 bytes:

```
challenge-hex + LF + "ScreenRing acknowledgement v1" + LF + complete-sent-packet
```

`complete-sent-packet` includes the request HMAC, but not its final LF. The prefix
separates response authentication from request authentication. Compare in constant
time. Reject a bare `OK`, wrong key, different challenge or different packet. This
rule applies to both v1 and v2 packets; there is no insecure fallback.

Update the TV receiver and KFritz together with the phone sender. Older receivers
replying OK cannot confirm delivery to an updated sender. Older senders expecting
OK also need updating. Prepare all components before replacing installed versions.
No new pairing key is required solely for this protocol change.

Packet fields:

```
v2<TAB>device-id<TAB>event<TAB>call-id<TAB>base64-envelope<TAB>hex-hmac
```

Device/call IDs contain 1–64 ASCII letters, digits or hyphens. Event is `RING`,
`CONNECT`, `END` or `TEST`. Device + call ID identify a call; repeated RING and
CONNECT messages update its lease and metadata, not its notification/history count.
END ends only that call. TEST shows a connection preview and never pauses playback.
No audio is part of the protocol.

The outer HMAC-SHA256 uses the decoded pairing key and authenticates:

```
challenge-hex + LF + packet-without-final-tab-and-hmac
```

Compare in constant time. The binary envelope contains a fresh 12-byte AES-GCM IV,
UTF-8 ciphertext and a 16-byte authentication tag, concatenated in that order.
Derive the AES-256 key as HMAC-SHA256(pairing-key-bytes, ASCII `ScreenRing v2 payload`).
GCM additional authenticated data is the challenge, LF and the first four packet
fields joined with TAB. The plaintext contains three UTF-8 fields separated by LF:

```
phone-label + LF + contact-name + LF + caller-number
```

Label is required, max 60 UTF-16 code units. Name and number are optional, max 120
and 64 respectively. No control characters are accepted. Absent caller data stays
empty; do not invent a number or associate an ambiguous SIM's contact.

Legacy v1 uses the same six outer fields, but field 5 is only a base64 UTF-8 phone
label and has no caller metadata. Updated receivers accept v1; v1 must never carry
caller details. A new v2 sender requires an updated receiver (ScreenRing 0.4+).

Each destination has a separate worker/acknowledgement state. An unreachable PC
must not delay the TV. Active calls refresh every 20 seconds; receiver leases expire
after 90 seconds without a refresh. A short state may be superseded before delivery
while offline; ended calls must never be reintroduced by old queued RING messages.

`tests/wire-v2.json` is an independent synthetic interoperability vector, also used
by Android tests. Its key and caller data are public test values, not user data.

Firewall: permit TCP 10123 only from the intended home network, not the internet.
A receiver must still require its pairing key; network location alone is not auth.
