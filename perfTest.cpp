#include <random>

#include "rtacl.hpp"
#include "cbProf.hpp"

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

int
main (int argc, char *argv[])
{
    cbProf::prof prof[4];
    size_t i;
    prof[0].setBanner("insert: ");
    prof[1].setBanner("match: ");
    prof[2].setBanner("unmatch: ");
    prof[3].setBanner("remove: ");
    for (i = 0; i < elementsof(prof); ++i) {
        prof[i].run();
    }

    rtacl::db<rtacl::ipv4a>    acl;
    rtacl::entry<rtacl::ipv4a> rtaclEnt;
    rtacl::sockItem<sockaddr_in> sKey;
    static rtacl::sockEnt<sockaddr_in>* pEnt[1000000]; // 1M entries
    //static rtacl::sockEnt<sockaddr_in>* pEnt[1000]; // 1K entries

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
    for (i = 0; i < elementsof(pEnt); ++i) {
        pEnt[i] = new(rtacl::sockEnt<sockaddr_in>);
        assert(pEnt[i]);

        rtacl::sockItem<sockaddr_in>& smin = pEnt[i]->getMin();
        rtacl::sockItem<sockaddr_in>& smax = pEnt[i]->getMax();
        ipv4a sa = 0x0a000000 + (i * 0x20);

        /*
         * minimum (lower bound)
         */
        siSrc.sin_addr.s_addr = htonl(sa);
        siSrc.sin_port        = htons(0);
        siDst.sin_addr.s_addr = htonl(0);
        siDst.sin_port        = htons(0);
        smin.set(siSrc, siDst, 6, 0);

        /*
         * maximum (upper bound)
         */
        siSrc.sin_addr.s_addr = htonl(sa + 10);
        siSrc.sin_port        = htons(65535);
        siDst.sin_addr.s_addr = htonl(0xffffffff);
        siDst.sin_port        = htons(65535);
        smax.set(siSrc, siDst, 6, 0xff);

        acl.makeMin(smin.getSrc(), smin.getDst(),
                    smin.getProto(), smin.getDSCP(), min);
        acl.makeMax(smax.getSrc(), smax.getDst(),
                    smax.getProto(), smax.getDSCP(), max);
        rtaclEnt.second = reinterpret_cast<uintptr_t>(pEnt[i]);
        prof[0].begin();
        acl.insert(rtaclEnt);
        prof[0].end();
    }
    std::cout << (bfmt("size: %ld, i: %ld\n") % acl.size() % i).str();
    assert(i == acl.size());
    prof[0].makeHist();
    std::cout << (bfmt("%s\n") % prof[0].str()).str();

#if 0
    /*
     * Show all ACL entries
     */
    rtacl::result<rtacl::ipv4a> r = acl.dump();
    for (auto it : r) {
        showRTaclEnt(it.first, it.second);
    }
#endif

#ifdef RUN_SEQUENTIAL_MATCH_TEST
    /*
     * ACL match test (match)
     */
    for (i = 0; i < elementsof(pEnt); ++i) {
        ipv4a sa = 0x0a000000 + (i * 0x20) + 2;

        siSrc.sin_addr.s_addr = htonl(sa);
        siDst.sin_addr.s_addr = htonl(0x12345678);
        siSrc.sin_port = htons(0x1234);
        siDst.sin_port = htons(80);
        sKey.set(siSrc, siDst, 6, 0);

        rtacl::tuple<rtacl::ipv4a> rtaclKey;
        acl.makeKey(sKey.getSrc(), sKey.getDst(),
                    sKey.getProto(), sKey.getDSCP(), rtaclKey);
        prof[1].begin();
        rtacl::result<rtacl::ipv4a> result = acl.find(rtaclKey);
        prof[1].end();
        if (result.size() != 1) {
            std::cout << "Error: no match: key: ";
            showSockItem(sKey);
        }
        int j = 0;
        for (auto r : result) {
            assert(reinterpret_cast<uintptr_t>(pEnt[i]) == r.second);
#if 0
            std::cout << (bfmt("pEnt: %p, result: 0x%lx\n")
                          % pEnt[i]
                          % r.second).str();
#endif//0
            ++j;
        }
    }
    prof[1].makeHist();
    std::cout << (bfmt("%s\n") % prof[1].str()).str();

    /*
     * ACL match test (unmatch)
     */
    for (i = 0; i < elementsof(pEnt); ++i) {
        ipv4a sa = 0x0a000000 + (i * 0x20) - 1 ;

        siSrc.sin_addr.s_addr = htonl(sa);
        siDst.sin_addr.s_addr = htonl(0x12345678);
        siSrc.sin_port = htons(0x1234);
        siDst.sin_port = htons(80);
        sKey.set(siSrc, siDst, 6, 0);

        rtacl::tuple<rtacl::ipv4a> rtaclKey;
        acl.makeKey(sKey.getSrc(), sKey.getDst(),
                    sKey.getProto(), sKey.getDSCP(), rtaclKey);
        prof[2].begin();
        rtacl::result<rtacl::ipv4a> result = acl.find(rtaclKey);
        prof[2].end();
        if (result.size() == 0) {
#if 0
            std::cout << "no match (correct): key: ";
            showSockItem(sKey);
#endif//0
        } else {
            std::cout << "Error: matched: key:";
            showSockItem(sKey);
            for (auto it : result) {
                showRTaclEnt<rtacl::ipv4a, sockaddr_in>(it.first, it.second);
            }
        }
    }
    prof[2].makeHist();
    printf("%s\n", prof[2].getCstr());
#endif//RUN_SEQUENTIAL_MATCH_TEST


    /*
     * Random match test
     */
    std::mt19937::result_type seed = time(NULL);
    auto mt_rand = std::bind(
        std::uniform_int_distribution<int>(0, elementsof(pEnt) - 1),
        std::mt19937(seed));

    prof[1].init();
    prof[1].run();
    for (i = 0; i < elementsof(pEnt); ++i) {
        u32 n = mt_rand();
        ipv4a sa = 0x0a000000 + (n * 0x20) + 2;

        siSrc.sin_addr.s_addr = htonl(sa);
        siDst.sin_addr.s_addr = htonl(0x12345678);
        siSrc.sin_port = htons(0x1234);
        siDst.sin_port = htons(80);
        sKey.set(siSrc, siDst, 6, 0);

        rtacl::tuple<rtacl::ipv4a> rtaclKey;
        acl.makeKey(sKey.getSrc(), sKey.getDst(),
                    sKey.getProto(), sKey.getDSCP(), rtaclKey);
        prof[1].begin();
        rtacl::result<rtacl::ipv4a> result = acl.find(rtaclKey);
        prof[1].end();
        if (result.size() != 1) {
            std::cout << "Error: no match: key: ";
            showSockItem(sKey);
        }
        int j = 0;
        for (auto r : result) {
            assert(reinterpret_cast<uintptr_t>(pEnt[n]) == r.second);
#if 0
            std::cout << (bfmt("pEnt: %p, result: 0x%lx\n")
                          % pEnt[n]
                          % r.second).str();
#endif//0
            ++j;
        }
    }
    prof[1].makeHist();
    std::cout << (bfmt("Random match test:\n%s\n") % prof[1].str()).str();

    /*
     * Random unmatch test
     */
    prof[2].init();
    prof[2].run();
    for (i = 0; i < elementsof(pEnt); ++i) {
        u32 n = mt_rand();
        ipv4a sa = 0x0a000000 + (n * 0x20) - 1 ;

        siSrc.sin_addr.s_addr = htonl(sa);
        siDst.sin_addr.s_addr = htonl(0x12345678);
        siSrc.sin_port = htons(0x1234);
        siDst.sin_port = htons(80);
        sKey.set(siSrc, siDst, 6, 0);

        rtacl::tuple<rtacl::ipv4a> rtaclKey;
        acl.makeKey(sKey.getSrc(), sKey.getDst(),
                    sKey.getProto(), sKey.getDSCP(), rtaclKey);
        prof[2].begin();
        rtacl::result<rtacl::ipv4a> result = acl.find(rtaclKey);
        prof[2].end();
        if (result.size() == 0) {
#if 0
            std::cout << "no match (correct): key: ";
            showSockItem(sKey);
#endif//0
        } else {
            std::cout << "Error: matched: key: ";
            showSockItem(sKey);
            for (auto it : result) {
                showRTaclEnt<rtacl::ipv4a, sockaddr_in>(it.first, it.second);
            }
        }
    }
    prof[2].makeHist();
    std::cout << (bfmt("Random unmatch test:\n%s\n") % prof[2].str()).str();

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
        rtaclEnt.second = reinterpret_cast<uintptr_t>(pEnt[i]);
        prof[3].begin();
        bool rc = acl.remove(rtaclEnt);
        prof[3].end();
        if (rc) {
#if 0
            std::cout << (bfmt("size: %2ld: ") % acl.size()).str();
#endif//0
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
#if 0
                std::cout << "no match (correct): key: ";
                showSockItem(sKey);
#endif//0
            } else {
                std::cout << "Error: matched: key: ";
                showSockItem(sKey);
                for (auto it : result) {
                    showRTaclEnt<rtacl::ipv4a, sockaddr_in>(it.first, it.second);
                }
            }
        } else {
            std::cout << "Error: failed to remove acl entry: ";
            showRTaclEnt<rtacl::ipv4a,
                         sockaddr_in>(rtaclEnt.first, rtaclEnt.second);
        }
    }
    prof[3].makeHist();
    std::cout << prof[3].str() << "\n";

    exit(0);
    return 0;
}
