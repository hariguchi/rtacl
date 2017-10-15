#include "rtacl.hpp"

using bfmt = boost::format;

/**
 * @name  showRTaclEnt
 * @brief Show RTree entry and the associated ACL entry
 *
 * @param ADDR  Must be either `sockaddr_in' or `sockaddr_in6'
 * @param SADDR Must be either `rtacl::ipv4' or `rtacl::ipv6'
 *
 * @param[in] r     Reference to an ACL range in `rtacl::range<ADDR>'
 * @param[in] index Poiner to `rtacl::sockEnt<SADDR>' in `uintptr_t'
 */
template <class ADDR, class SADDR>
static void
showRTaclEnt (const rtacl::range<ADDR>& r, const uintptr_t index)
{
    rtacl::sockEnt<SADDR>* p =
        reinterpret_cast<rtacl::sockEnt<SADDR>*>(index);

    std::cout << (bfmt("rtree: %s\nacl:   %s\n")
                  % rtacl::range2str(r)
                  % p->str()).str();
}

/**
 * @name  showSockItem
 * @brief Show the content of `rtacl::sockItem<SADDR>'
 *
 * @param SADDR Must be either `sockaddr_in' or `sockaddr_in6'
 *
 * @param[in] i Reference to `rtacl::sockItem<SADDR>'
 */
template <class SADDR>
static void
showSockItem (rtacl::sockItem<SADDR>& i)
{
    std::cout << (bfmt("sockItem: %s\n") % i.str()).str();
}

/**
 * @name  v4test
 * @brief R-tree ACL functional test (IPv4)
 */
static void
v4sockTest()
{
    rtacl::db<rtacl::ipv4a>    acl;
    rtacl::entry<rtacl::ipv4a> rtaclEnt;
    rtacl::sockItem<sockaddr_in> sKey;
    std::unique_ptr<rtacl::sockEnt<sockaddr_in> > pEnt[10];

    rtacl::tuple<rtacl::ipv4a>& min = rtaclEnt.first.min_corner();
    rtacl::tuple<rtacl::ipv4a>& max = rtaclEnt.first.max_corner();

    sockaddr_in siSrc;
    sockaddr_in siDst;
    memset(&siSrc, 0, sizeof(siSrc));
    memset(&siDst, 0, sizeof(siDst));
    siSrc.sin_family = AF_INET;
    siDst.sin_family = AF_INET;

    /*
     * Add ACL entries
     */
    size_t i;
    for (i = 0; i < elementsof(pEnt); ++i) {
        pEnt[i].reset(new rtacl::sockEnt<sockaddr_in>);
        assert(pEnt[i]);

        rtacl::sockItem<sockaddr_in>& smin = pEnt[i]->getMin();
        rtacl::sockItem<sockaddr_in>& smax = pEnt[i]->getMax();
        ipv4a sa = 0x0a0a0a00 + (i * 20);

        /*
         * minimum (lower bound)
         */
        siSrc.sin_addr.s_addr = htonl(sa);
        siSrc.sin_port        = htons(0);
        siDst.sin_addr.s_addr = htonl(0);
        siDst.sin_port        = htons(80);
        smin.set(siSrc, siDst, 6, 0);

        /*
         * maximum (upper bound)
         */
        siSrc.sin_addr.s_addr = htonl(sa + 10);
        siSrc.sin_port        = htons(65535);
        siDst.sin_addr.s_addr = htonl(0xffffffff);
        siDst.sin_port        = htons(80);
        smax.set(siSrc, siDst, 6, 0xff);

        acl.makeMin(smin.getSrc(), smin.getDst(),
                    smin.getProto(), smin.getDSCP(), min);
        acl.makeMax(smax.getSrc(), smax.getDst(),
                    smax.getProto(), smax.getDSCP(), max);
        rtaclEnt.second = reinterpret_cast<uintptr_t>(pEnt[i].get());
        acl.insert(rtaclEnt);
    }
    std::cout << (bfmt("size: %ld, i: %ld\n") % acl.size() % i).str();
    assert(i == acl.size());

    /*
     * Show all ACL entries
     */
    rtacl::result<rtacl::ipv4a> r = acl.dump();
    for (auto it : r) {
        showRTaclEnt<rtacl::ipv4a, sockaddr_in>(it.first, it.second);
    }

    /*
     * ACL match test (match)
     */
    for (i = 0; i < elementsof(pEnt); ++i) {
        ipv4a sa = 0x0a0a0a00 + (i * 20) + 2;

        siSrc.sin_addr.s_addr = htonl(sa);
        siDst.sin_addr.s_addr = htonl(0x12345678);
        siSrc.sin_port = htons(0x1234);
        siDst.sin_port = htons(80);
        sKey.set(siSrc, siDst, 6, 0);
        rtacl::tuple<rtacl::ipv4a> rtaclKey;
        acl.makeKey(sKey.getSrc(), sKey.getDst(),
                    sKey.getProto(), sKey.getDSCP(), rtaclKey);
        rtacl::result<rtacl::ipv4a> result = acl.find(rtaclKey);
        if (result.size() != 1) {
            std::cout << "Error: no match. Key:\n";
            showSockItem(sKey);
        }
        int j = 0;
        for (auto r : result) {
            assert(reinterpret_cast<uintptr_t>(pEnt[i].get()) == r.second);
            std::cout << (bfmt("pEnt: %p, result: 0x%lx\n")
                          % pEnt[i].get()
                          % r.second).str();
            ++j;
        }
    }

    /*
     * ACL match test (unmatch)
     */
    for (i = 0; i < elementsof(pEnt); ++i) {
        ipv4a sa = 0x0a0a0a00 + (i * 20) - 1 ;

        siSrc.sin_addr.s_addr = htonl(sa);
        siDst.sin_addr.s_addr = htonl(0x12345678);
        siSrc.sin_port = htons(0x1234);
        siDst.sin_port = htons(80);
        sKey.set(siSrc, siDst, 6, 0);
        rtacl::tuple<rtacl::ipv4a> rtaclKey;
        acl.makeKey(sKey.getSrc(), sKey.getDst(),
                    sKey.getProto(), sKey.getDSCP(), rtaclKey);
        rtacl::result<rtacl::ipv4a> result = acl.find(rtaclKey);
        if (result.size() == 0) {
            std::cout << "no match (correct): key: ";
            showSockItem(sKey);
        } else {
            std::cout << "Error: matched: key: ";
            showSockItem(sKey);
            for (auto it : result) {
                showRTaclEnt<rtacl::ipv4a,
                             sockaddr_in>(it.first, it.second);
            }
        }
    }

    /*
     * Remove ACL entries
     */
    for (i = 0; i < elementsof(pEnt); ++i) {
        rtacl::sockItem<sockaddr_in>& smin = pEnt[i]->getMin();
        rtacl::sockItem<sockaddr_in>& smax = pEnt[i]->getMax();
        acl.makeMin(smin.getSrc(), smin.getDst(),
                    smin.getProto(), smin.getDSCP(), min);
        acl.makeMax(smax.getSrc(), smax.getDst(),
                    smax.getProto(), smax.getDSCP(), max);
        rtaclEnt.second = reinterpret_cast<uintptr_t>(pEnt[i].get());
        bool rc = acl.remove(rtaclEnt);
        if (rc) {
            std::cout << (bfmt("size: %2ld: ") % acl.size()).str();
            ipv4a a = ntohl(smin.getSrc().sin_addr.s_addr) + 1;
            siSrc.sin_addr.s_addr = htonl(a);
            siDst.sin_addr.s_addr = smin.getDst().sin_addr.s_addr;
            siSrc.sin_port = htons(smin.getSrc().sin_port);
            siDst.sin_port = htons(smin.getDst().sin_port);
            sKey.set(siSrc, siDst, smin.getProto(), smin.getDSCP());
            rtacl::tuple<rtacl::ipv4a> rtaclKey;
            acl.makeKey(sKey.getSrc(), sKey.getDst(),
                        sKey.getProto(), sKey.getDSCP(), rtaclKey);
            rtacl::result<rtacl::ipv4a> result = acl.find(rtaclKey);
            if (result.size() == 0) {
                std::cout << "no match (correct): key: ";
                showSockItem(sKey);
            } else {
                std::cout << "Error: matched: key: ";
                showSockItem(sKey);
                for (auto it : result) {
                    showRTaclEnt<rtacl::ipv4a,
                                 sockaddr_in>(it.first, it.second);
                }
            }
        } else {
            std::cout << "Error: failed to remove acl entry: ";
            showRTaclEnt<rtacl::ipv4a,
                         sockaddr_in>(rtaclEnt.first, rtaclEnt.second);

        }
    }
}

/**
 * @name  v6test
 * @brief R-tree ACL functional test (IPv6)
 */
static void
v6sockTest()
{
    rtacl::db<rtacl::ipv6a>    acl;
    rtacl::entry<rtacl::ipv6a> rtaclEnt;
    rtacl::sockItem<sockaddr_in6> sKey;
    std::unique_ptr<rtacl::sockEnt<sockaddr_in6> > pEnt[10];
    rtacl::tuple<rtacl::ipv6a>& min = rtaclEnt.first.min_corner();
    rtacl::tuple<rtacl::ipv6a>& max = rtaclEnt.first.max_corner();

    sockaddr_in6 siSrc;
    sockaddr_in6 siDst;
    memset(&siSrc, 0, sizeof(siSrc));
    memset(&siDst, 0, sizeof(siDst));
    siSrc.sin6_family = AF_INET6;
    siDst.sin6_family = AF_INET6;

    /*
     * Add ACL entries
     */
    size_t i;
    for (i = 0; i < elementsof(pEnt); ++i) {
        pEnt[i].reset(new rtacl::sockEnt<sockaddr_in6>);
        assert(pEnt[i].get());

        rtacl::sockItem<sockaddr_in6>& smin = pEnt[i]->getMin();
        rtacl::sockItem<sockaddr_in6>& smax = pEnt[i]->getMax();
        u32 a = 0x0a0a0a00 + (i * 20);

        /*
         * minimum (lower bound)
         */
        siSrc.sin6_addr.s6_addr[0] = 0x20;
        siSrc.sin6_addr.s6_addr[1] = 0x01;
        siSrc.sin6_addr.s6_addr[6] = 0x11;
        siSrc.sin6_addr.s6_addr[7] = 0x11;
        *((u32*)(siSrc.sin6_addr.s6_addr + 12)) = htonl(a);
        siSrc.sin6_port        = htons(0);
        memset(siDst.sin6_addr.s6_addr, 0, sizeof(siDst.sin6_addr.s6_addr));
        siDst.sin6_port        = htons(80);
        smin.set(siSrc, siDst, 6, 0);

        /*
         * maximum (upper bound)
         */
        *((u32*)(siSrc.sin6_addr.s6_addr + 12)) = htonl(a + 10);
        siSrc.sin6_port = htons(65535);
        memset(siDst.sin6_addr.s6_addr,
               0xff, sizeof(siDst.sin6_addr.s6_addr));
        siDst.sin6_port = htons(80);
        smax.set(siSrc, siDst, 6, 0xff);

        acl.makeMin(smin.getSrc(), smin.getDst(),
                    smin.getProto(), smin.getDSCP(), min);
        acl.makeMax(smax.getSrc(), smax.getDst(),
                    smax.getProto(), smax.getDSCP(), max);
        rtaclEnt.second = reinterpret_cast<uintptr_t>(pEnt[i].get());
        acl.insert(rtaclEnt);
    }
    std::cout << (bfmt("size: %ld, i: %ld\n") % acl.size() % i).str();
    assert(i == acl.size());

    /*
     * Show all ACL entries
     */
    rtacl::result<rtacl::ipv6a> r = acl.dump();
    for (auto it : r) {
        showRTaclEnt<rtacl::ipv6a, sockaddr_in6>(it.first, it.second);
    }
    /*
     * ACL match test (match)
     */
    for (i = 0; i < elementsof(pEnt); ++i) {
        /*
         * Source key
         */
        u32 a = 0x0a0a0a00 + (i * 20) + 2;
        siSrc.sin6_addr.s6_addr[0] = 0x20;
        siSrc.sin6_addr.s6_addr[1] = 0x01;
        siSrc.sin6_addr.s6_addr[6] = 0x11;
        siSrc.sin6_addr.s6_addr[7] = 0x11;
        u32 na = htonl(a);
        memcpy(siSrc.sin6_addr.s6_addr + 12, &na, sizeof(na));
        siSrc.sin6_port = htons(0x1234);
        siDst.sin6_port = htons(80);
        sKey.set(siSrc, siDst, 6, 0);
        /*
         * Destination key
         */
        siDst.sin6_addr.s6_addr[0]  = 0x20;
        siDst.sin6_addr.s6_addr[1]  = 0x00;
        siDst.sin6_addr.s6_addr[2]  = 0x00;
        siDst.sin6_addr.s6_addr[3]  = 0x01;
        siDst.sin6_addr.s6_addr[15] = 0x01;
        memset(siDst.sin6_addr.s6_addr + 4, 0, 11);
        sKey.set(siSrc, siDst, 6, 0);

        rtacl::tuple<rtacl::ipv6a> rtaclKey;
        acl.makeKey(sKey.getSrc(), sKey.getDst(),
                    sKey.getProto(), sKey.getDSCP(), rtaclKey);
        rtacl::result<rtacl::ipv6a> result = acl.find(rtaclKey);
        if (result.size() != 1) {
            std::cout << "Error: no match. key: ";
            showSockItem(sKey);
        }
        int j = 0;
        for (auto r : result) {
            assert(reinterpret_cast<uintptr_t>(pEnt[i].get()) == r.second);
            std::cout << (bfmt("pEnt: %p, result: 0x%lx\n")
                          % pEnt[i].get()
                          % r.second).str();
            ++j;
        }
    }

    /*
     * ACL match test (unmatch)
     */
    for (i = 0; i < elementsof(pEnt); ++i) {
        /*
         * Source key
         */
        u32 a = 0x0a0a0a00 + (i * 20) - 1;
        siSrc.sin6_addr.s6_addr[0] = 0x20;
        siSrc.sin6_addr.s6_addr[1] = 0x01;
        siSrc.sin6_addr.s6_addr[6] = 0x11;
        siSrc.sin6_addr.s6_addr[7] = 0x11;
        u32 na = htonl(a);
        memcpy(siSrc.sin6_addr.s6_addr + 12, &na, sizeof(na));
        siSrc.sin6_port = htons(0x1234);
        siDst.sin6_port = htons(80);
        sKey.set(siSrc, siDst, 6, 0);
        /*
         * Destination key
         */
        siDst.sin6_addr.s6_addr[0]  = 0x20;
        siDst.sin6_addr.s6_addr[1]  = 0x00;
        siDst.sin6_addr.s6_addr[2]  = 0x00;
        siDst.sin6_addr.s6_addr[3]  = 0x01;
        siDst.sin6_addr.s6_addr[15] = 0x01;
        memset(siDst.sin6_addr.s6_addr + 4, 0, 11);
        sKey.set(siSrc, siDst, 6, 0);

        rtacl::tuple<rtacl::ipv6a> rtaclKey;
        acl.makeKey(sKey.getSrc(), sKey.getDst(),
                    sKey.getProto(), sKey.getDSCP(), rtaclKey);
        rtacl::result<rtacl::ipv6a> result = acl.find(rtaclKey);
        if (result.size() == 0) {
            std::cout << "no match (correct): key: ";
            showSockItem(sKey);
        } else {
            std::cout << "Error: matched: key: ";
            showSockItem(sKey);
            for (auto it : result) {
                showRTaclEnt<rtacl::ipv6a, sockaddr_in6>(it.first, it.second);
            }
        }
    }
    /*
     * Remove ACL entries
     */
    for (i = 0; i < elementsof(pEnt); ++i) {
        rtacl::sockItem<sockaddr_in6>& smin = pEnt[i]->getMin();
        rtacl::sockItem<sockaddr_in6>& smax = pEnt[i]->getMax();
        acl.makeMin(smin.getSrc(), smin.getDst(),
                    smin.getProto(), smin.getDSCP(), min);
        acl.makeMax(smax.getSrc(), smax.getDst(),
                    smax.getProto(), smax.getDSCP(), max);
        rtaclEnt.second = reinterpret_cast<uintptr_t>(pEnt[i].get());
        bool rc = acl.remove(rtaclEnt);
        if (rc) {
            std::cout << (bfmt("size: %2ld: ") % acl.size()).str();

            sKey = smin;
            sKey.getSrc().sin6_addr.s6_addr[15] += 1;

            rtacl::tuple<rtacl::ipv6a> rtaclKey;
            acl.makeKey(sKey.getSrc(), sKey.getDst(),
                        sKey.getProto(), sKey.getDSCP(), rtaclKey);

            rtacl::result<rtacl::ipv6a> result = acl.find(rtaclKey);
            if (result.size() == 0) {
                std::cout << "no match (correct): key: ";
                showSockItem(sKey);
            } else {
                std::cout << "Error: matched: key: ";
                showSockItem(sKey);
                for (auto it : result) {
                    showRTaclEnt<rtacl::ipv6a,
                                 sockaddr_in6>(it.first, it.second);
                }
            }
        } else {
            std::cout << "Error: failed to remove acl entry: ";
            showRTaclEnt<rtacl::ipv6a,
                         sockaddr_in6>(rtaclEnt.first, rtaclEnt.second);
        }
    }
}

/**
 * @name  v4rawTest
 * @brief R-tree ACL functional test (IPv4)
 */
static void
v4rawTest ()
{
    /*
     * Sample ACL tuple and ACL entry (host byte order)
     */
    struct aclTuple {
        ipv4a sa;               // source IPv4 address
        ipv4a da;               // destination IPv4 address
        u16 sp;                 // source port number
        u16 dp;                 // destination port number
        u8 proto;               // IP protocol (TCP, UDP, etc.)
        u8 dscp;                // DSCP
    };
    struct aclEnt {
        aclTuple min;           // lower bound
        aclTuple max;           // upper bound
    };

    aclEnt ent[10];
    rtacl::db<rtacl::ipv4a> acl;
    rtacl::entry<rtacl::ipv4a> rtAclEnt;

    /*
     * Insert 10 ACL entries into R-tree
     */
    memset(ent, 0, sizeof(ent));
    size_t i;
    for (i = 0; i < elementsof(ent); ++i) {
        rtacl::tuple<rtacl::ipv4a>& min = rtAclEnt.first.min_corner();
        rtacl::tuple<rtacl::ipv4a>& max = rtAclEnt.first.max_corner();

        ent[i].min.sa = 0x0a010000 + (i << 8); // 10.1.i.0
        ent[i].max.sa = ent[i].min.sa + 10;    // 10.1.i.10
        ent[i].min.da = 0;
        ent[i].max.da = ~0;
        ent[i].min.sp = 0;
        ent[i].max.sp = ~0;
        ent[i].min.dp = 80;     // http
        ent[i].max.dp = 80;
        ent[i].min.proto = 6;   // TCP
        ent[i].max.proto = 6;
        ent[i].min.dscp = 0;
        ent[i].max.dscp = ~0;
        /*
         * sa: 10.1.i.0 - 10.1.i.10,
         * da: 0.0.0.0  - 255.255.255.255
         * sp: 0 - 65535
         * dp: 80
         * proto: TCP (6)
         * dscp: 0 - 255
         */
        acl.makeMin(ent[i].min.sa, ent[i].min.da,
                    ent[i].min.sp, ent[i].min.dp,
                    ent[i].min.proto, ent[i].min.dscp, min);
        acl.makeMax(ent[i].max.sa, ent[i].max.da,
                    ent[i].max.sp, ent[i].max.dp,
                    ent[i].max.proto, ent[i].max.dscp, max);
        rtAclEnt.second = reinterpret_cast<uintptr_t>(ent + i);
        acl.insert(rtAclEnt);
    }
    std::cout << (bfmt("size: %ld, i: %ld\n") % acl.size() % i).str();
    assert(i == acl.size());
    /*
     * Show all ACL entries
     */
    rtacl::result<rtacl::ipv4a> r = acl.dump();
    for (auto it : r) {
        rtacl::entry<rtacl::ipv4a> e;
        rtacl::tuple<rtacl::ipv4a>& min = e.first.min_corner();
        rtacl::tuple<rtacl::ipv4a>& max = e.first.max_corner();
        aclEnt* p = reinterpret_cast<aclEnt*>(it.second);
        acl.makeMin(p->min.sa, p->min.da,
                    p->min.sp, p->min.dp,
                    p->min.proto, p->min.dscp, min);
        acl.makeMax(p->max.sa, p->max.da,
                    p->max.sp, p->max.dp,
                    p->max.proto, p->max.dscp, max);
        std::cout << (bfmt("rtree: %s\nacl:   %s\n")
                      % rtacl::range2str(it.first)
                      % rtacl::range2str(e.first)).str();
    }
    /*
     * ACL match test (match)
     */
    for (i = 0; i < elementsof(ent); ++i) {
        rtacl::tuple<rtacl::ipv4a> key;
        acl.makeKey(ent[i].min.sa + 1, ent[i].min.da,
                    ent[i].min.sp, ent[i].min.dp,
                    ent[i].min.proto, ent[i].min.dscp, key);
        rtacl::result<rtacl::ipv4a> result = acl.find(key);
        if (result.size() != 1) {
            std::cout << "Error: no match. key: ";
            std::cout << (bfmt("%s\n") % rtacl::tuple2str(key)).str();
        }
        for (auto r : result) {
            assert(reinterpret_cast<uintptr_t>(ent + i) == r.second);
            std::cout << (bfmt("ent: %p, result: 0x%lx\n")
                          % (ent + i)
                          % r.second).str();
        }
    }
    /*
     * ACL match test (unmatch)
     */
    for (i = 0; i < elementsof(ent); ++i) {
        rtacl::tuple<rtacl::ipv4a> key;
        acl.makeKey(ent[i].min.sa + 11, ent[i].min.da,
                    ent[i].min.sp, ent[i].min.dp,
                    ent[i].min.proto, ent[i].min.dscp, key);
        rtacl::result<rtacl::ipv4a> result = acl.find(key);
        if (result.size() == 0) {
            std::cout << "no match (correct): key: ";
            std::cout << (bfmt("%s\n") % rtacl::tuple2str(key)).str();
        } else {
            std::cout << "Error: matched: key: ";
            std::cout << (bfmt("%s\n") % rtacl::tuple2str(key)).str();

            for (auto it : result) {
                std::cout << (bfmt("%s, %p\n")
                              % rtacl::range2str(it.first)
                              % it.second).str();
            }
        }
    }
    /*
     * Remove ACL entries
     */
    for (i = 0; i < elementsof(ent); ++i) {
        rtacl::entry<rtacl::ipv4a> e;
        rtacl::tuple<rtacl::ipv4a>& min = e.first.min_corner();
        rtacl::tuple<rtacl::ipv4a>& max = e.first.max_corner();
        acl.makeMin(ent[i].min.sa, ent[i].min.da,
                    ent[i].min.sp, ent[i].min.dp,
                    ent[i].min.proto, ent[i].min.dscp, min);
        acl.makeMax(ent[i].max.sa, ent[i].max.da,
                    ent[i].max.sp, ent[i].max.dp,
                    ent[i].max.proto, ent[i].max.dscp, max);
        e.second = reinterpret_cast<uintptr_t>(ent + i);
        bool rc = acl.remove(e);
        if (rc) {
            std::cout << (bfmt("size: %2ld: ") % acl.size()).str();

            rtacl::tuple<rtacl::ipv4a> key;
            acl.makeKey(ent[i].min.sa + 1, ent[i].min.da,
                        ent[i].min.sp, ent[i].min.dp,
                        ent[i].min.proto, ent[i].min.dscp, key);
            rtacl::result<rtacl::ipv4a> result = acl.find(key);
            if (result.size() == 0) {
                std::cout << (bfmt("no match (correct): key: %s\n")
                              % rtacl::tuple2str(key)).str();
            } else {
                std::cout << (bfmt("Error: matched: key: %s\n")
                              % rtacl::tuple2str(key)).str();
                for (auto it : result) {
                    std::cout << (bfmt("%s, %p\n")
                                  % rtacl::range2str(it.first)
                                  % it.second).str();
                }
            }
        } else {
            std::cout << (bfmt("Error: failed to remove acl entry: %s %ld\n")
                          % rtacl::range2str(e.first)
                          % e.second).str();
        }
    }
}


int
main (int argc, char *argv[])
{
    std::cout << "IPv4 Raw Test\n";
    v4rawTest();
    std::cout << "\nIPv4 sockaddr Test\n";
    v4sockTest();
    std::cout << "\nIPv6 sockaddr Test\n";
    v6sockTest();
}
