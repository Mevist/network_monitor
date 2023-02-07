
#include <arpa/inet.h>
#include <linux/rtnetlink.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <net/if.h>

#define IFI_RTA(r)  ((struct rtattr*)(((char*)(r)) + NLMSG_ALIGN(sizeof(struct ifinfomsg))))
#define IFI_PAYLOAD(n) NLMSG_PAYLOAD(n,sizeof(struct ifinfomsg))

#define ND_RTA(r)  ((struct rtattr*)(((char*)(r)) + NLMSG_ALIGN(sizeof(struct ndmsg))))
#define ND_PAYLOAD(n) NLMSG_PAYLOAD(n,sizeof(struct ndmsg))


int main(int argc, char **argv) {
  int sfd, rclen, nllen, atlen;
  char buf[8192];
  char *ptr;
  char addr[32], dst[32], msk[32], gwy[32], brd[32], dev[32], nddst[32], ndladdr[32];
  unsigned int mtu;
  struct sockaddr_nl snl;
  struct nlmsghdr *nlp;
  struct ifinfomsg *riftp;
  struct ifaddrmsg *raddrtp;
  struct rtmsg *rtp;
  struct ndmsg *ndtp;
 // struct nda_cacheinfo *ndctp;
  struct rtattr *atp;
  int ifmsg_flag, ifaddr_flag, route_flag, nd_flag;


  sfd = socket(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE);
  memset(&snl, 0, sizeof(struct sockaddr_nl));
  snl.nl_family = AF_NETLINK;
  snl.nl_pid = getpid();
  snl.nl_groups = RTMGRP_LINK | RTMGRP_IPV4_IFADDR | RTMGRP_IPV4_ROUTE | RTMGRP_NEIGH | RTMGRP_NOTIFY;
  bind(sfd, (struct sockaddr*) &snl, sizeof(struct sockaddr_nl));

  while(1) {
    memset(&buf, 0, sizeof(buf));
    ptr = buf;
    nllen = 0;
    do {
      rclen = recv(sfd, ptr, sizeof(buf) - nllen, 0);
      nlp = (struct nlmsghdr*) ptr;
      ptr += rclen;
      nllen += rclen;
    } while(nlp->nlmsg_type == NLMSG_DONE);

    nlp = (struct nlmsghdr*) buf;
    for(;NLMSG_OK(nlp, nllen); nlp = NLMSG_NEXT(nlp, nllen)) {
      riftp = (struct ifinfomsg*) NLMSG_DATA(nlp);
      atp = (struct rtattr*) IFI_RTA(riftp);
      atlen = IFI_PAYLOAD(nlp);
      ifmsg_flag = RTA_OK(atp, atlen);

      raddrtp = (struct ifaddrmsg*) NLMSG_DATA(nlp);
      atp = (struct rtattr*) IFA_RTA(raddrtp);
      atlen = IFA_PAYLOAD(nlp);
      ifaddr_flag = RTA_OK(atp, atlen);

      rtp = (struct rtmsg*) NLMSG_DATA(nlp);
      atp = (struct rtattr*) RTM_RTA(rtp);
      atlen = RTM_PAYLOAD(nlp);
      route_flag = RTA_OK(atp, atlen);

      ndtp = (struct ndmsg*) NLMSG_DATA(nlp);
      atp = (struct rtattr*) ND_RTA(ndtp);
      atlen = ND_PAYLOAD(nlp);
      nd_flag = RTA_OK(atp, atlen);

     // printf("FLAG MSG %d\nFLAG ADDR %d\nFLAG ROUTE %d\n",ifmsg_flag, ifaddr_flag, route_flag);
      if (ifmsg_flag && (nlp->nlmsg_type == RTM_DELLINK || nlp->nlmsg_type == RTM_NEWLINK))
      {
          // printf("IFMSG FLAG!!\n");
          atp = (struct rtattr*) IFI_RTA(riftp);
          atlen = IFI_PAYLOAD(nlp);
          memset(dev, 0, sizeof(dev));
          for(;RTA_OK(atp, atlen); atp = RTA_NEXT(atp, atlen) ) {
            switch(atp-> rta_type) {
              case IFLA_MTU:        memcpy(&mtu, RTA_DATA(atp),
                                            sizeof(mtu));
                                  break;        
              case IFLA_IFNAME:     memcpy(&dev, RTA_DATA(atp),
                                        sizeof(dev));
                                  break;
            }
        }

        if (nlp->nlmsg_type == RTM_DELLINK)
          printf("[DEL] ");
        else if (nlp->nlmsg_type == RTM_NEWLINK){
          printf("[ADD] ");
            if (riftp->ifi_flags > 30000){
              printf("Interface: | %s | was turned | ON |, it's MTU is: | %u |\n", dev, mtu);
              }
            else
              printf("Interface: | %s | was turned | OFF |, it's MTU is: | %u |\n", dev, mtu);
        }
        //____________________________________________________________//
      }
      if (ifaddr_flag && (nlp->nlmsg_type == RTM_DELADDR || nlp->nlmsg_type == RTM_NEWADDR))
      {
        atp = (struct rtattr*) IFA_RTA(raddrtp);
        atlen = IFA_PAYLOAD(nlp);
        memset(addr, 0, sizeof(addr));
        memset(brd, 0, sizeof(brd));
        memset(dev, 0, sizeof(dev));
        for(;RTA_OK(atp, atlen); atp = RTA_NEXT(atp, atlen) ) {
            switch(atp-> rta_type) {
              case IFA_ADDRESS:    inet_ntop(AF_INET, RTA_DATA(atp), addr,
                                        sizeof(addr));
                                  break;
              case IFA_BROADCAST:  inet_ntop(AF_INET, RTA_DATA(atp), brd,
                                        sizeof(brd));
                                  break;
              case IFA_LABEL:     memcpy(&dev, RTA_DATA(atp),
                                        sizeof(dev));
                                  break;
            }
        }
        printf("print type: %d\n", nlp->nlmsg_type);
        if (nlp->nlmsg_type == RTM_DELADDR)
          printf("[DEL] ");
        else if (nlp->nlmsg_type == RTM_NEWADDR)
          printf("[ADD] ");
        printf("Interface: | %s | with address | %s |, and it's broadcast address: | %s |\n", dev, addr, brd);
      }
      //____________________________________________________________//
      if(route_flag && (nlp->nlmsg_type == RTM_DELROUTE || nlp->nlmsg_type == RTM_NEWROUTE))
      {
        atp = (struct rtattr*) RTM_RTA(rtp);
        atlen = RTM_PAYLOAD(nlp);
        memset(dst, 0, sizeof(dst));
        memset(msk, 0, sizeof(msk));
        memset(gwy, 0, sizeof(gwy));
        memset(dev, 0, sizeof(dev));
        for(;RTA_OK(atp, atlen); atp = RTA_NEXT(atp, atlen)) {
          switch(atp-> rta_type) {
            case RTA_DST:     inet_ntop(AF_INET, RTA_DATA(atp), dst,
                                        sizeof(dst));
                              break;
            case RTA_GATEWAY: inet_ntop(AF_INET, RTA_DATA(atp), gwy,
                                        sizeof(gwy));
                              break;
            case RTA_OIF:     sprintf(dev, "%d", *((int*) RTA_DATA(atp)));
                              break;
          }
        }
        sprintf(msk, "%X", rtp->rtm_dst_len);
        if (nlp->nlmsg_type == RTM_DELROUTE)
          printf("[DEL] ");
        else if (nlp->nlmsg_type == RTM_NEWROUTE)
          printf("[ADD] ");
        if (strlen(dst) == 0)
          printf("default via %s dev %s\n", gwy, dev);
        else if (strlen(gwy) == 0)
          printf("%s/%s dev %s\n", dst, msk, dev);
        else
          printf("dst %s/%s gwy %s dev %s\n", dst, msk, gwy, dev);
      }
      //____________________________________________________________//
      if(nd_flag && (nlp->nlmsg_type == RTM_DELNEIGH || nlp->nlmsg_type == RTM_NEWNEIGH))
      {
        atp = (struct rtattr*) ND_RTA(ndtp);
        atlen = ND_PAYLOAD(nlp);
        memset(nddst, 0, sizeof(nddst));
        memset(ndladdr, 0, sizeof(ndladdr));

        for(;RTA_OK(atp, atlen); atp = RTA_NEXT(atp, atlen)) {
          printf("TEST!!!\n\n");
          switch(atp-> rta_type) {
            case NDA_DST:     inet_ntop(AF_INET, RTA_DATA(atp), nddst,
                                        sizeof(nddst));
                              break;
            case NDA_LLADDR:  sprintf(ndladdr, "%x", *((int*) RTA_DATA(atp)));
                              break;
          }
        }
      if (nlp->nlmsg_type == RTM_DELNEIGH)
          printf("[DEL] ");
      else if (nlp->nlmsg_type == RTM_NEWNEIGH)
          printf("[ADD] ");
      printf("Interface index: | %d | with layer destination address | %s |, and link layer addres: | %s |", ndtp->ndm_ifindex, nddst, ndladdr);
      }
      //____________________________________________________________//
    }
  }
  close(sfd);
  return EXIT_SUCCESS;
}
