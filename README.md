# OP_Y2S2M3
# Network Connection and Transaction System Simulation
*
### running 
*
Open STNC in client normal chat mode.
`./stnc -c <IP> <PORT>`

Open STNC in server normal chat mode.
`./stnc -s <PORT>`

 Open STNC in client performance mode.
`./stnc -c <IP> <PORT> -p <TYPE> <PARAM>`

 Open STNC in server performance mode.
`./stnc -s <PORT> -p`

Open STNC in server performance and quiet mode.
`./stnc -s <PORT> -p -q`

*
In computer networking, data integrity is crucial for reliable data transmission. One of the methods to ensure data integrity is through checksums. Checksum is a numerical value calculated from a data packet used for error detection. In this project, we will discuss how to calculate the checksum for different protocols such as IPv4 TCP, IPv4 UDP, IPv6 TCP, IPv6 UDP, Unix domain socket datagram (UDS DGRAM), Unix domain socket stream (UDS STREAM), Memory Mapped Files (MMAP), and Named Pipes.
*
Network Performance Tool
*

* IPv4 TCP

This protocol is used to establish a reliable connection between two devices on the internet. 
*
*  IPv4 UDP

This protocol is used for faster transmission of data between two devices on the internet. 
*
*
* IPv6 TCP

This protocol is used to establish a reliable connection between two devices on the internet.
*
* IPv6 UDP

This protocol is used for faster transmission of data between two devices on the internet. 
*
* UDS Datagram

This protocol is used for communication between processes on the same machine. 
*
* UDS Stream

This protocol is also used for communication between processes on the same machine. 
*
* Memory-mapped files

This technique is used to access data in memory and map it to a file on disk. 
*
* pipes

Transfer the generated data via piping (local machine only). 
*
* Chat cmd 

The tool is a command-line chat application that can send messages to another instance of the tool running on a remote network and receive responses back, allowing for two-way communication. This communication is established using the Internet Protocol version 4 (IPv4) and the Transmission Control Protocol (TCP) protocol.
