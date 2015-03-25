# dhcp_test_tool

## Introdcution
DHCP server test tool by MFC

It could be used to test the performance of DHCP server.

## Attention
I completed it very long ago as one fresh developer, so it is very simple, but useful.
Becasue there is no similar open-source tool as far as I know.

## Background
I took charge of developing the DHCP server when I got my first job. One task is that enhance the performance of DHCP server.
So I develop this tool to get the performance result of DHCP server and other features with my private time at home. Then its copyright belongs to myself, not My company.

## Usage

I expains some configures which seems not clear
### General Setting
Local IP: It's the local IP of the computer which runs the tool. It should be gotten by tool normally, please correct it if not right.
Server IP: The IP of DHCP server
Interval: The interval of that tool sends the DHCPDISCOVER
Total Time: The time that the test last
Random MAC: The tool will fill the random mac field of DHCP packet 
Sorted MAC: The tool will fill the mac field of DHCP packet sequenctly from 00:00:00:00:00:01
Fixed MAC: The tool will fill the mac field of DHCP packet with the specific value

### Advanced Setting
Subnet: Specify the subnet which is used to allocate IPs from DHCP server. Normally the client only allocates the IP with same subnet with the interface of DHCP server which receives the DHCPDISCOVER. You could use this field to get the IPs of other subnet.
Fixed cnt: The tool will allocate the IPs by specific count
Repeat MAC: Sorry, I forget the accurate meaning. You could get it by reading codes:D






