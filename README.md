
# RougeFi
## Deauthentication Attack

The deauthentication attack exploits vulnerabilities in the IEEE 802.11 protocol, which governs wireless communication. It works by sending deauthentication frames to forcibly disconnect clients from an access point (AP). These frames are unencrypted and can be easily spoofed, allowing attackers to impersonate the AP or the client. The attack disrupts network connections, leaving devices disconnected and vulnerable to further exploitation, such as Man-in-the-Middle (MITM) attacks or phishing.
Deauthentication frames are legitimate management frames used to signal a client that its connection to the AP has been terminated. However, attackers misuse this feature by sending numerous spoofed deauth frames, often targeting all devices connected to an AP, to induce chaos or prepare for subsequent malicious actions.

## ESP8266 and Its Capabilities
The ESP8266 microcontroller is a low-cost, highly capable Wi-Fi chip that includes a full TCP/IP stack and microcontroller functionality. Its ability to perform packet injection makes it ideal for security research and demonstrating protocol vulnerabilities.

By using the Wi-Fi_send_pkt_freedom function, the ESP8266 can inject custom packets, including spoofed deauth frames, into a network. While this feature enables precise control over packet crafting, it has limitations, such as restrictions on packet size and payload customization, which can influence the attack's execution.

## Packet Injection
Packet injection is a process where custom-crafted packets are introduced into a network stream. The ESP8266's capability for packet injection allows it to send deauthentication frames and other custom packets, which are critical for executing attacks like deauth and phishing. Although powerful, packet injection on the ESP8266 may require careful optimization due to its hardware limitations.

## Mitigation via IEEE 802.11w
The IEEE 802.11w-2009 standard addresses the vulnerabilities exploited by deauthentication attacks by introducing Protected Management Frames (PMF). PMF encrypts management frames, preventing spoofing and ensuring the authenticity of such communications. Despite its effectiveness, PMF adoption has been slow, and many networks remain vulnerable.



## Features

- Deauthentication of Specific Channel/Targt
- User Friendly Interface
- Failsafe 



## Run Locally

Clone the project

```bash
  git clone https://github.com/Karxthi/RougeFi
```

Go to the project directory

```bash
  cd RougeFi
```

Install dependencies



## Acknowledgements

 - [Deauth Attack](https://mrncciew.com/2014/10/11/802-11-mgmt-deauth-disassociation-frames/)
 - [Packet Injection ](http://hackaday.com/2016/01/14/inject-packets-with-an-esp8266/)
 - [Examples](https://github.com/pulkin/esp8266-injection-example)

