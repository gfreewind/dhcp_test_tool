# dhcp_test_tool

## Introdcution
DHCP server test tool by MFC

It could be used to test the performance of DHCP server.


## Gossip  
Actually I am one Linux programmer :)   
I completed this very long ago as one fresh developer, so it is very simple, but a little useful. It was born in 2007 year and ther was no similar open-source tool as far as I know. It seems there are some tools now.  
Anyway, I also share it with everybody today.   
It was saved in Google codes before. I export it to github today because of the coming end of google codes.

## Background
I took charge of developing the DHCP server when I got my first job. One task is that enhance the performance of DHCP server.
I develop this tool in my private time to get the performance result of DHCP server and test other features easily. 
So its copyright belongs to myself, not the company.

## Usage

I expains some configures which seems not clear now.
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


## Sources Directory   
bin: There is one execute binary of this tool. You could run it directly.  
Because I wanted to share some codes with different projects, so I created the inc and utils directories as the most outside directory.  
The dhcp_tool saves the project file and source codes of DHCP performance tool.

