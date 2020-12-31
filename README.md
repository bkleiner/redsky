# redsky

An implementation of the Redpine rc protocol (by BryceJ) tailored to run on Frsky xm and xm+ and similar receivers.

redsky is a super low latency radio control protocol with excellent signal robustness even in challenging environments.
It can be easily flashed to suitable receivers via most radio transmitters and is reversible should it be required.

Flashing instructions for OpenTX radios: 
 1. download the .frk file that matches your receiver
 2. copy to the sd card from your radio (usually a folder called "Firmware")
 3. connect a servo plug or similar to the reciever power, ground and s-port and connect to s-port or jr bay pins in radio
 (include picture or link to flashing article, Oscar Liang etc)
 4. turn on radio and enter system menu
 5. page over to find the Firmware folder and click on the .frk file you downloaded
 6. choose option "flash external module" and click enter, flash should complete without error
 7. turn off radio and remove reciever

Binding:
