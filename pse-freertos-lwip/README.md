This program implements a simple web server example using FreeRTOS and the
LwIP TCP/IP stack with the PSE MSS MAC Ethernet driver. This can be used to
exercise the driver on the HW Emulation platform with the Peripheral
Daughter Board, Renode PolarFire SoC emulation or the FU540 based Aloe board.
 

The software has a fixed IP address of **172.16.0.3**/255.255.255.0
Opening this address with a web-browser should display a simple web page.
 

The IP address can be change from within this file:
*/src/modules/config/lwip-2.0.0-wip/network_interface_settings.h*
 
When targeting the Renode emulation then the messages on the UART show what
extra steps are required for the bridge between the host and the emulation
to work properly on the Linux host (no Windows support with SoftConsole 6.1).

When creating the bridge for the first time, it might display error:
```
Could not set TUNSETIFF, error: 2
```
Renode is trying to probe the user's privileges, which should be safe to ignore.
If the polkit and sudo are set to be passwordless then the bridge should be
created correctly without any other user's interaction.  

When targeting Renode, make sure that the IP settings are not overlapping with
existing network on your host as the *renode-tap0* requires to be own network.

When enabling Wireshark logger, some extra steps might be required to allow
regular users to capture network traffic (read the materials listed below).
 
When targeting real HW then the IP network should match existing real network
and just the IP address itself needs to be unique (join an existing network, but
do not create conflicts).


For more details read the pse-blinky's readme, SoftConsole Release Notes,
Renode's manual and visit:
https://www.microsemi.com/product-directory/fpga-soc/5210-mi-v-embedded-ecosystem#renode-webinar-series

**Note:** The example currently is not using the most up-to-date HAL release,
the GEM driver required some changes to be made on top of HAL so it is not
straight forward to port to the newest HAL. This will be addressed in the SoftConsole 6.2
release, however in the meantime use the bundled pse-blinky/pse-mustein-julia or firmware catalog
examples use the most recent HAL.
