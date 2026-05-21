/* Common file for server & client */

#include "tftp.h"
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>

extern int Data_size;
extern int mode_flag;
extern int mode_enable ;
char prev = 0;
int prev_nline = 0 ;

void send_file(int sockfd, struct sockaddr_in client_addr, socklen_t client_len, char *filename)
{
    // Packet used to send DATA or ERROR
    tftp_packet send_packet;

    // Packet used to receive ACK or ERROR
    tftp_packet rec_packet;

    int r_ret, Send_data_bytes, data_pack_count = 0;

    // Open file in read-only mode
    int fd = open(filename, O_RDONLY, 0777);

    // If file open fails
    if (fd < 0)
    {
        perror("open");

        // Prepare ERROR packet
        send_packet.opcode = ERROR;

        strcpy(send_packet.body.error_packet.error_msg, "Server File Don't have Read permission");

        // Send ERROR to client
        sendto(sockfd, &send_packet, sizeof(send_packet), 0, (struct sockaddr *)&client_addr, client_len);

        return;
    }

    do
    {
        // Clear send packet and receive packet before filling
        memset(&send_packet, 0, sizeof(send_packet));
        memset(&rec_packet , 0 , sizeof(rec_packet));

        // If netascii mode, convert before sending
        if (mode_flag == 3)
         r_ret = convert_netascii(fd, send_packet.body.data_packet.data);

        // Otherwise read normally
        else
            r_ret = read(fd, send_packet.body.data_packet.data, Data_size);
        printf("mode flag :%d\n",mode_flag);
        printf("%d bytes readed from the file\n", r_ret);

        // Set opcode as DATA
        send_packet.opcode = DATA;

        // Increment block number
        data_pack_count++;

        send_packet.body.data_packet.block_number = data_pack_count;

        // Store number of bytes in this packet
        send_packet.body.data_packet.nof_data_bytes = r_ret;

        // Send DATA packet
        Send_data_bytes = sendto(sockfd, &send_packet,sizeof(send_packet), 0, (struct sockaddr *)&client_addr,client_len);

        printf("%d  bytes sent  \n", Send_data_bytes);

        // Wait for ACK from client
        recvfrom(sockfd, &rec_packet,sizeof(rec_packet), 0, NULL, NULL);

        // If client sends ERROR
        if (rec_packet.opcode == ERROR)
        {
            printf("Error ->%s\n",rec_packet.body.error_packet.error_msg);

            // Reposition file pointer to resend same block
            lseek(fd, -r_ret, SEEK_CUR);

            // Decrement block counter to resend
            data_pack_count--;
        }

        // If ACK received
        else if (rec_packet.opcode == ACK)
        {
            printf("Ack received for the data packet %d\n",rec_packet.body.ack_packet.block_number);
        }

    }
    // Continue until last packet size < Data_size
    while (r_ret == Data_size);

    close(fd);
    mode_enable = 0;

    printf("File sent to the client Successfully\n");
}

void receive_file(int sockfd, struct sockaddr_in client_addr,socklen_t client_len, char *filename)
{
    // Packet to send ACK or ERROR
    tftp_packet send_packet = {0};

    // Packet to receive DATA
    tftp_packet rec_packet = {0};

    int data_pkt_count = 0, w_ret = 0;

    int Rec_data_byts;

    // Open file in write mode (create or truncate)
    int fd = open(filename,O_WRONLY | O_CREAT | O_TRUNC, 0777);

    // If file open fails
    if (fd < 0)
    {
        perror("open");
        printf("file is not open for writing\n ");
        return;
    }
     printf("fd = %d\n",fd);
    do
    {
        // To clear both packets
        memset(&send_packet, 0, sizeof(send_packet));
        memset(&rec_packet , 0 , sizeof(rec_packet));
        // Receive DATA packet from sender
        int Rec_data_byts = recvfrom(sockfd, &rec_packet,sizeof(rec_packet),0, NULL, NULL);

        printf("%d bytes data received \n", Rec_data_byts);

        // Increment expected block number
        data_pkt_count++;

        // If block number matches expected value
        if (rec_packet.body.data_packet.block_number == data_pkt_count)
        {
            
           // If packet is DATA
           if (rec_packet.opcode == DATA)

            // Write received data into file
            printf("fd = %d\n",fd);
            w_ret = write(fd , rec_packet.body.data_packet.data ,  rec_packet.body.data_packet.nof_data_bytes);
            printf("no of bytes rec %d\n",rec_packet.body.data_packet.nof_data_bytes) ;
            printf("and data :%s\n",rec_packet.body.data_packet.data);

             printf("%d bytes data written in file \n", w_ret);
            // Prepare ACK packet
            send_packet.opcode = ACK;

            send_packet.body.ack_packet.block_number = data_pkt_count;
        }

        else
        {
            // Prepare ERROR packet for mismatch
            send_packet.opcode = ERROR;

            strcpy(send_packet.body.error_packet.error_msg, "Data Packet block number  mismatch");
        }

        // Send ACK or ERROR back to sender
        sendto(sockfd, &send_packet, sizeof(send_packet),0, (struct sockaddr *)&client_addr,client_len);

    }
    // Continue until last data block size < Data_size
    while (w_ret == Data_size);

    printf("Loop breaked\n");
    mode_enable = 0;

    close(fd);
}

// int convert_netascii(int fd, char buff[]) // it will  return how many bytes readed
// {
//     int i = 0, j = 0;
//     int ret = 1;
//     char ch =0;
//     if(prev_nline)
//     {
//         buff[j++]='\n' ;
//         prev_nline = 0 ;
//     }
//     while (j < 512 && ret != 0)
//     {
//         ret = read(fd, &ch, 1);
//         printf("in the  net ascii function ->char ->%c readed from the file\n",ch) ;
//         if (ret == 0)
//             break;
//         if (ch == '\n')
//         {
//             if (prev == '\r')
//             {
//                 if(j > 0)
//                  buff[--j] = ch;
//                  else
//                  {
//                     buff[j] = ch ;  
//                  }
//             }   
//             else
//             {
//                 buff[j++] = '\r'  ;
//                 if(j < 512)
//                 buff[j++] =  '\n' ;                // to check  if last char is '/r' in previous buffer then how to add '\n' at  0 th index
//                 else
//                 prev_nline = 1 ;          // To add '\n'  in next data  packet
//             }
//         }
//             else buff[j++] = ch;

//             prev = ch;
//         }
//         return j;
//     }

int convert_netascii(int fd, char buff[])   // returns number of bytes read
{
    int j = 0;
    int ret = 1;
    char ch = 0;

    while (j < 512 && ret != 0)
    {
        ret = read(fd, &ch, 1);
        if (ret <= 0)
            break;

        // If previous char was '\r' then check current character 
        if (prev == '\r')
        {
            if (ch == '\n')
            {
                // Found \r\n  → convert to single '\n'
                buff[j++] = '\n';
                prev = 0;
                continue;
            }
            else
            {
                // '/r' alone (there is no '\n' with it)
                buff[j++] = '\r';
                prev = 0;
                
            }
        }

        if (ch == '\r')
        {
            // Don't write yet, wait to see next char
            prev = '\r';
        }
        else if (ch == '\n')
        {
            // cuurent char is  '\n' → convert to '\r\n'
            if (j + 2 <= 512)
            {
                buff[j++] = '\r';
                buff[j++] = '\n';
            }
            else
            {
                // Not enough space, stop here
                break;
            }
        }
        else
        {
            buff[j++] = ch;
        }
    }

    return j;
}

