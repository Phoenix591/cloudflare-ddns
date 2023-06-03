This is my simple cloudflare dynamic dns app. 
It reads a small config file  (~/.cloudflare/ddns.cfg) and iterates through all subdomains and the root and points them to your external ip address.

if useIPv6 is set it will do both A and AAAA records. 

Example config
```
token = CLOUDFLARE_TOKEN
zone_name = example.com
subdomains = a b
useIPv6 = 0
```

Requirements:    
dig, libcurl, jsoncpp
