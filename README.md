# crypto-iot
Crypto Authentication for IoT

You can test it at http://sha204a.cf

demo key = **"EB0C68BF96E8C26635D3450293D2FC501A63A09924FE90A7BD916AC521FDE0AA"**

http request = `"GET /?timestamp=1458647701 HTTP/1.1\r\n"`

add header:
 ```
Authorization: 11PATHS-HMAC-256 id="EjEjEg==",
                                nonce="LmzzEpRnXvqmvnbOSobGp1VysR/wEpWoMNaY2Miew5g=",
                                base="EQACAAAAAAAAAAAAAAAA7gAAAAABIwAA",
                                signature="4qnOa5ZGecdzC+DscOSuOhJ64LeB1jTieJATUWPoIZE="
 ```
response will be like this:

 ```
 {
    "responsejson": {
        "emulator": {
            "version": "0.3",
            "contact": "http://github.com/latchdevel/crypto-iot"
        },
        "server": {
            "scheme": "http",
            "host": "sha204a.cf",
            "method": "GET",
            "uri": "/?timestamp=1458647701"
        },
        "auth": {
            "type": "11PATHS-HMAC-256",
            "id": "12312312",
            "nonce": "2E6CF31294675EFAA6BE76CE4A86C6A75572B11FF01295A830D698D8C89EC398",
            "base": "110002000000000000000000000000EE0000000001230000",
            "signature": "E2A9CE6B964679C7730BE0EC70E4AE3A127AE0B781D634E27890135163E82191"
        },
        "timestamp": {
            "request": "Tue, 22 Mar 2016 12:55:01 +0100",
            "current": "Tue, 22 Mar 2016 12:54:52 +0100",
            "drift": 0
        },
        "crypt": {
            "request": "GET /?timestamp=1458647701 HTTP/1.1\r\n",
            "req-sha1": "649B5E6834E206FEFE3110366C814BAA3795EAD5",
            "hmac-256": "E2A9CE6B964679C7730BE0EC70E4AE3A127AE0B781D634E27890135163E82191",
            "authenticated": true
        },
        "result": "(It will be a surprise)"
    }
}
 ```
