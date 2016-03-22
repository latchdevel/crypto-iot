#!/bin/bash

KEY="EB0C68BF96E8C26635D3450293D2FC501A63A09924FE90A7BD916AC521FDE0AA"

KEY_BIN="`echo -en "$KEY"|xxd -r -p`"

ID="12312313"
ZEROS="0000000000000000000000000000000000000000000000000000000000000000"
BASE="110002000000000000000000000000EE0000000001230000"

NONCE="`openssl rand 32 -hex`"
NONCE_BASE="160000"

echo "TEST hmac mode 0"
echo ""
echo "TempKey  =  SHA-256(32bytes RandOut & 20bytes SHA1    & 1byte Opcode=0x16 & 1byte Mode & 1byte LSB-Param2=0x00)"
echo "HMAC-256 = HMAC-256(32byres Zeros   & 32bytes TempKey & 24bytes base)"
echo ""

Server="http://sha204a.cf"
Method="GET"
URL="/"
TIMESTAMP="?timestamp=`date +%s`"

AuthHeader="Authorization: "
AuthHeader+="11PATHS-HMAC-256 "
AuthHeader+="id=\"`echo -en "$ID"|xxd -r -p|base64`\","
AuthHeader+="nonce=\"`echo -en "$NONCE"|xxd -r -p|base64`\","
AuthHeader+="base=\"`echo -en "$BASE"|xxd -r -p|base64`\","

REQSHA="`echo -en "$Method $URL$TIMESTAMP HTTP/1.1\r\n" | openssl dgst -sha1 | cut -d" " -f2`"
TOSHA="$NONCE$REQSHA$NONCE_BASE"
TEMPKEY="`echo -en "$TOSHA" | xxd -r -p | openssl dgst -sha256 | cut -d" " -f2 `"
TOHMAC="$ZEROS$TEMPKEY$BASE"

echo "NONCE = $NONCE"
echo "R-SHA = $REQSHA"
echo "TOSHA = $TOSHA"
echo "TEMPK = $TEMPKEY"
echo "TOHMAC= $TOHMAC"

Signature="`echo -en "$TOHMAC" | xxd -r -p | openssl dgst -sha256 -hmac "$KEY_BIN" | cut -d" " -f2`"

echo "HMAC  = $Signature"

# Warning:
# - for OpenSSL 0.9.8x like as MacOSX: openssl dgst -sha256 -mac hmac -macopt hexkey:"$KEY"
# - for OpenSSL 1.x.x use standart:    openssl dgst -sha256 -hmac "$KEY_BIN"

AuthHeader+="signature=\"`echo -en "$Signature"|xxd -r -p|base64`\""
UserAgent="User-Agent: Mozilla/5.0"

echo "$AuthHeader"

Response="`curl -q -s -N -k --header "$UserAgent" --header "$AuthHeader" "$Server$URL$TIMESTAMP"`"

echo "$Response"
