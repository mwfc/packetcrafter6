/* This is a very basic program to show the use of cmsg and setting Hoplimit. */

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/uio.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>

// TODO Flowlabel, Checksum, Extensionheaders, src address spoofing
// Works: hoplimit (socket/packet)
// TODO: restructure to a lib to become useful later
// TODO: clean memory mallocs
// TODO: go SOCK_RAW?
// TODO: Error checking...
// TODO: Doxify

int main(void) {

	int sock;
	// packet
	struct sockaddr_in6 from, to;
	memset(&to, 0, sizeof(from));
	memset(&from, 0, sizeof(to));

	struct iovec iov;
	struct msghdr msg;
	struct cmsghdr *cmsg;
	int cmsglen; //total length of all cmsgs

	memset(&cmsg, 0, sizeof(struct cmsghdr));


	// Testing Values HEADER for loopback
	int hoplimit = 123;
	char payload[] = "This is just some MARKER";
	to.sin6_port = htons(1234);
	to.sin6_family = AF_INET6;
	inet_pton(to.sin6_family, "::1", &to.sin6_addr);

	from.sin6_port = htons(4321);
	from.sin6_family = AF_INET6;
	inet_pton(from.sin6_family, "::1", &from.sin6_addr);

	// normal packet building
	iov.iov_base = &payload;
	iov.iov_len = sizeof(payload);

	msg.msg_name = &to;
	msg.msg_namelen = sizeof(struct sockaddr_in6);
	msg.msg_iov = &iov;
	msg.msg_iovlen = 1;

	// control header fun starts here
	cmsglen = CMSG_SPACE (sizeof (int));
	msg.msg_control = (unsigned char *) malloc (cmsglen * sizeof(unsigned char));
	if (msg.msg_control == NULL){
		perror("Could not allocate memory for cmsglen");
		exit(1);
	 }
	 memset (msg.msg_control, 0, cmsglen);
	 msg.msg_controllen = cmsglen;


	cmsg = CMSG_FIRSTHDR (&msg);


	set_hoplimitpkt(cmsg, hoplimit);

	sock = socket(AF_INET6, SOCK_DGRAM, 0);


	if(bind(sock, (struct sockaddr *)&from, sizeof(struct sockaddr_in6)) == -1){
		perror("Socket binding failed");
		exit(1);
	}


	if (sendmsg(sock, &msg, 0) == -1) {
		perror("Sending packet failed");
	}
	printf("Packet sent. \n");

	// Free memory
	free(msg.msg_control);

	return EXIT_SUCCESS;
}



// Works, but modifies whole socket
int set_hoplimit(int sock, int hoplimit){
	if ( setsockopt(sock, IPPROTO_IPV6, IPV6_UNICAST_HOPS, &hoplimit, sizeof(hoplimit)) == -1){
		fprintf(stderr, "Could not set Hoplimit");
		return -1;
	}
	return 0;
}

// Works, but modifies only one packet
int set_hoplimitpkt(struct cmsghdr* cmsg, int hoplimit){
	cmsg->cmsg_level = IPPROTO_IPV6;
	cmsg->cmsg_type = IPV6_HOPLIMIT;
	cmsg->cmsg_len = CMSG_LEN (sizeof (int));
	*((int *) CMSG_DATA (cmsg)) = hoplimit;
	return 0;
}

/* TODO
// Normally you retrieve flowlabels and register them with the kernel, we just create them. On a per Socket basis does not sound right though.
int set_flowinfo(int sock, struct sockaddr_in6 *addr ){
	int on = 1;
	setsockopt(sock, IPPROTO_IPV6, IPV6_FLOWINFO_SEND, on, sizeof(on)); // now you can set flowlabels
	setsockopt(sock, IPPROTO_IPV6, IPV6_FLOWLABEL_MGR, in6_flowlabel_req, sizeof(struct in6_flowlabel_req));
	return 0;
}
*/


// TODO
int set_checksum(){
	return 0;
}

// TODO spoof src ip
/* Needs to kill some checks for binds */
