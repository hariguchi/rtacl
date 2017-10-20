#ifndef __RTACL_HPP__
#define __RTACL_HPP__

/*
 * Copyright (c) 2017 Yoichi Hariguchi
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated
 * documentation files (the "Software"), to deal in the
 * Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute,
 * sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so,
 * subject to the following conditions: 
 *
 * The above copyright notice and this permission notice shall
 * be included in all copies or substantial portions of the
 * Software. 
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY
 * KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
 * PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS
 * OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. 
 *
 * R-tree ACL (Access Control List)
 *
 * This is an ACL implementation using R-tree
 * (boost::geometry::index::rtree)
 * R-tree is a data structure for indexing multi-dimensional
 * information like polygons. ACL is equivalent to find a
 * 5-dimensional box (ranges of 5 tuples) in the tree wherein
 * the search key is a 5-dimensional point (5-tuple.)
 * The original R-tree paper can be retrieved from here:
 *   http://www-db.deis.unibo.it/courses/SI-LS/papers/Gut84.pdf
 *
 * See unitTest.cpp for examples (for both IPv4 and IPv6.)
 *
 */

#include "local_types.h"

#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <arpa/inet.h>

#include <boost/geometry.hpp>
#include <boost/geometry/geometries/point.hpp>
#include <boost/geometry/geometries/box.hpp>
#include <boost/geometry/index/rtree.hpp>
#include <boost/tuple/tuple_comparison.hpp>
#include <boost/tuple/tuple_io.hpp>
#include <boost/format.hpp>

#include <vector>
#include <iostream>

typedef u32  ipv4a;             // IPv4 address (host byte order)
typedef u128 ipv6a;             // IPv6 address (host byte order)

namespace rtacl {

enum
{
    dim = 6, // dimension: src IP, dest IP, src port, desr port, proto, dscp
    offsetKey = 0,
    offsetMin = -1,
    offsetMax = 1,
};

namespace bg  = boost::geometry;
namespace bgi = bg::index;

/**
 * @brief IP address types for rtree (host byte order)
 */
typedef s64  ipv4a;     // since rtree supports neither <= nor >=
typedef s256 ipv6a;     // since rtree supports neither <= nor >=

/**
 * @name  rtacl::tuple
 * @brief ACL tuple (search key)
 *        order: src IP, dst IP, src port, dst port, proto, dscp
 *        (all of them are in the host byte order)
 *        Boost R-tree supports neither >= nor <=.
 *        Hence 'any' is as follows:
 *          IPv6 address: (-1, 0x100000000000000000000000000000000)
 *          IPv4 address: (-1, 0x100000000)
 *          Port number:  (-1, 0x10000)
 *          IP proto:     (-1, 0x100)
 *
 * @param ADDR \e rtacl::ipv4a (\e s64) or \e rtacl::ipv6a (\e s256)
 */
template <class ADDR>
using tuple = bg::model::point<ADDR, dim, bg::cs::cartesian>;

/**
 * @name  rtacl::range
 * @brief ACL range
 *        Range is equivalent to box (N-dimensional rectangle)
 *
 * @param ADDR \ertacl::ipv4a (\e s64) or \e rtacl::ipv6a (\e s256)
 */
template <class ADDR>
using range = bg::model::box<tuple<ADDR> >;

/**
 * @name  rtacl::entry
 * @brief R-tree ACL entry
 *        The first parameter of the pair is an ACL range.
 *        The second parameter of the pair is an unique index
 *        associated with the first parameter. Usually the second
 *        parameter is a pointer to the user-defined ACL entry
 *        associated with the R-tree entry. Hence its type is
 *        \e uintptr_t.
 *        TODO: \e uintptr_t' can be a template parameter,
 *        but it createts compilation errors regarding
 *        \b boost::geometry::index::equal_to<>.
 *
 * @param ADDR \e rtacl::ipv4a (\e s64) or \e rtacl::ipv6a (\e s256)
 */
template <class ADDR>
using entry = std::pair<range<ADDR>, uintptr_t>;

/**
 * @name  rtacl::result
 * @brief ACL search result
 *
 * @param ADDR \e rtacl::ipv4a (\e s64) or \e rtacl::ipv6a (\e s256)
 */
template <class ADDR>
using result = std::vector<entry<ADDR> >;

/*
 * forward declaration
 */
template <class ADDR> class sockEnt;

/**
 * @class rtacl::sockItem
 * @brief Supporting data structure
 *        ACL item using \e sockaddr
 *
 * @param SADDR \e sockaddr_in or \e sockaddr_in6
 */
template <class SADDR>
class sockItem {
private:
    SADDR src __aligned;
    SADDR dst __aligned;
    sa_family_t af;           // copy of \e sin_family or \e sin6_family
    u16 ao;                   // offset to \e sin_addr' or \e sin6_addr
    u16 po;                   // offset to \e sin_port' or \e sin6_port
    u8 proto;
    u8 dscp;
public:
    sockItem ();
    void set (SADDR& src, SADDR& dst, u8 proto, u8 dscp) {
        this->src =   src;
        this->dst =   dst;
        this->proto = proto;
        this->dscp  = dscp;
    };
    void setSrc (SADDR& src) { this->src = src; };
    void setDst (SADDR& dst) { this->dst = dst; };
    void setProto (u8 proto) { this->proto = proto; };
    void setDSCP (u8 dscp) { this->dscp = dscp; };
    SADDR& getSrc () { return src; };
    SADDR& getDst () { return dst; };
    void* getsa () { return ((u8*)(&src) + ao); };
    void* getda () { return ((u8*)(&dst) + ao); };
    u16 getsp () { return ntohs(*((u16*)((u8*)(&src) + po))); };
    u16 getdp () { return ntohs(*((u16*)((u8*)(&dst) + po))); };
    u8 getProto () const { return proto; };
    u8 getDSCP () const { return dscp; };
    std::string str();

    friend class sockEnt<SADDR>;
};

/**
 * @class rtacl::sockEnt
 * @brief Supporting data structure
 *        ACL entry using \e sockaddr
 *
 * @param SADDR \e sockaddr_in or \e sockaddr_in6
 */
template <class SADDR>
class sockEnt {
private:
    sockItem<SADDR> min;
    sockItem<SADDR> max;
    u32 priority;      // tie breaker in the case of the multiple hits
public:
    void set (sockItem<SADDR>& min, sockItem<SADDR>& max, u32 pri) {
        this->min = min;
        this->max = max;
        priority = pri;
    };
    void setMin (sockItem<SADDR>& min) { this->min = min; };
    void setMax (sockItem<SADDR>& max) { this->max = max; };
    void setPriority (u32 pri) { priority = pri; };
    sockItem<SADDR>& getMin () { return min; };
    sockItem<SADDR>& getMax () { return max; };
    u32 getPriority () const { return priority; };
    std::string str();
};

/**
 * @class rtacl::db
 * @brief R-tree based ACL
 *
 * @param ADDR \e rtacl::ipv4a (\e s64) or \e rtacl::ipv6a (\e s256)
 */
template <class ADDR=rtacl::ipv4a>
class db
{
private:
    bgi::rtree<entry<ADDR>, bgi::quadratic<16> > rtree;
    sa_family_t af;           // copy of \e sin_family or \e sin6_family
    u16 ao;                   // offset to \e sin_addr or \e sin6_addr
    u16 po;                   // offset to \e sin_port or \e sin6_port
    u8 ipVer;
public:
    db();
    void insert(entry<ADDR> const& ent) { rtree.insert(ent); };
    bool remove(entry<ADDR> const& ent) {
        return ((rtree.remove(ent) > 0) ? true : false);
    };
    result<ADDR> find(const tuple<ADDR>& key);
    size_t size() const { return rtree.size(); };
    result<ADDR> dump() const;
    /*
     * helper functions
     */
    void makeMin (const sockaddr_in& src,
                 const sockaddr_in& dst,
                  const u8 proto,
                  const u8 dscp,
                  tuple<rtacl::ipv4a>& result) {
        makeTuple(src, dst, proto, dscp, offsetMin, result);
    }
    void makeMax (const sockaddr_in& src,
                  const sockaddr_in& dst,
                  const u8 proto,
                  const u8 dscp,
                  tuple<rtacl::ipv4a>& result) {
        makeTuple(src, dst, proto, dscp, offsetMax, result);
    }
    void makeKey (const sockaddr_in& src,
                  const sockaddr_in& dst,
                  const u8 proto,
                  const u8 dscp,
                  tuple<rtacl::ipv4a>& result) {
        makeTuple(src, dst, proto, dscp, offsetKey, result);
    }
    void makeMin (const sockaddr_in6& src,
                  const sockaddr_in6& dst,
                  const u8 proto,
                  const u8 dscp,
                  tuple<rtacl::ipv6a>& result) {
        makeTuple(src, dst, proto, dscp, offsetMin, result);
    }
    void makeMax (const sockaddr_in6& src,
                  const sockaddr_in6& dst,
                  const u8 proto,
                  const u8 dscp,
                  tuple<rtacl::ipv6a>& result) {
        makeTuple(src, dst, proto, dscp, offsetMax, result);
    }
    void makeKey (const sockaddr_in6& src,
                  const sockaddr_in6& dst,
                  const u8 proto,
                  const u8 dscp,
                  tuple<rtacl::ipv6a>& key) {
        makeTuple(src, dst, proto, dscp, offsetKey, key);
    }
    void makeMin (const ADDR sa,
                  const ADDR da,
                  const ADDR sp,
                  const ADDR dp,
                  const ADDR proto,
                  const ADDR dscp,
                  tuple<ADDR>& result) {
        makeTuple(sa, da, sp, dp, proto, dscp, offsetMin, result);
    }
    void makeMax (const ADDR sa,
                  const ADDR da,
                  const ADDR sp,
                  const ADDR dp,
                  const ADDR proto,
                  const ADDR dscp,
                  tuple<ADDR>& result) {
        makeTuple(sa, da, sp, dp, proto, dscp, offsetMax, result);
    }
    void makeKey (const ADDR sa,
                  const ADDR da,
                  const ADDR sp,
                  const ADDR dp,
                  const ADDR proto,
                  const ADDR dscp,
                  tuple<ADDR>& result) {
        makeTuple(sa, da, sp, dp, proto, dscp, offsetKey, result);
    }
private:
    void makeTuple(const sockaddr_in& src,
                   const sockaddr_in& dst,
                   const u8 proto,
                   const u8 dscp,
                   const s32 offset,
                   tuple<rtacl::ipv4a>& result);
    void makeTuple(const sockaddr_in6& src,
                   const sockaddr_in6& dst,
                   const u8 proto,
                   const u8 dscp,
                   const s32 offset,
                   tuple<rtacl::ipv6a>& result);
    void makeTuple(const ADDR sa,
                   const ADDR da,
                   const ADDR sp,
                   const ADDR dp,
                   const ADDR proto,
                   const ADDR dscp,
                   const s32 offset,
                   tuple<ADDR>& result);
};

/*
 * Non class member inline functions
 */

/**
 * @name  ipv4a2s
 * @brief Converts \e rtacl::ipv4a to \e std::string
 *
 * @param[in] a IPv4 address as \e rtacl::ipv4a
 *
 * @retval \e std::string IPv4 address as a string
 */
inline std::string
ipv4a2s (const rtacl::ipv4a a)
{
    std::string s;
    u64 mask = 0xff000000;
    u64 val;
    int i;
    for (i = 24; i > 0; i -= 8) {
        val = (a & mask) >> i;
        mask >>= 8;
        s += (boost::format("%d.") % val).str();
    }
    val = (a & mask);
    s += (boost::format("%d") % val).str();
    return s;
}

/**
 * @name  ipv6a2s
 * @brief Converts \e rtacl::ipv6a to \e std::string
 *
 * @param[in] a IPv6 address as \e rtacl::ipv6a
 *
 * @retval \e std::string IPv6 address as a string
 */
inline std::string
ipv6a2s (const rtacl::ipv6a& a)
{
    sockaddr_in6 sa;
    char buf[64];

    memset(&sa, 0, sizeof(sa));
    sa.sin6_family = AF_INET6;
    size_t max = elementsof(sa.sin6_addr.s6_addr) - 1;
    size_t i;
    for (i = 0; i <= max; ++i) {
        u8 n = static_cast<u8>(a >> (i << 3));
        sa.sin6_addr.s6_addr[max - i] = n;
    }
    inet_ntop(AF_INET6, &sa.sin6_addr, buf, sizeof(buf));
    std::string s(buf);
    return s;
}

/**
 * @name  sin6a2int
 * @brief Converts \e sockaddr_in6 to \e INT
 *
 * @param INT Must be either \e ipv6a (\e u128) or
 *            \e rtacl::ipv6a (\e u256)
 *
 * @param[in] a IPv6 address as \e sockaddr_in6
 *
 * @retval \e sin6.sin6_addr as \e INT
 */
template <class INT>
inline INT
sin6a2int (const sockaddr_in6& sin6)
{
    INT addr;
    size_t i;
    for (i = 0; i < elementsof(sin6.sin6_addr.s6_addr) - 1; ++i) {
        addr |= sin6.sin6_addr.s6_addr[i];
        addr <<= 8;
    }
    addr |= sin6.sin6_addr.s6_addr[i];
    return addr;
}

/**
 * @name  int2sin6
 * @brief Converts \b INT to \e sockaddr_in6
 *
 * @param INT Must be either \e ipv6a (\e u128) or
 *            \e rtacl::ipv6a (\e u256)
 *
 * @param[in] a IPv6 address as \e INT
 *
 * @retval IPv6 address as \e sockaddr_in6
 */
template <class INT>
inline sockaddr_in6
int2sin6 (const INT& addr)
{
    sockaddr_in6 sin6;
    size_t max = elementsof(sin6.sin6_addr.s6_addr) - 1;
    size_t i;
    for (i = 0; i <= max; ++i) {
        u8 n = static_cast<u8>(addr >> (i << 3));
        sin6.sin6_addr.s6_addr[max - i] = n;
    }
    return sin6;
}

/**
 * @name  tuple2str
 * @brief Converts \e rtacl::tuple<ipv4a> to \e std::string
 *
 * @param[in] t IPv4 6-tuple (sa, da, sp, dp, proto, dscp)
 *
 * @retval \b t as \e std::string
 */
inline std::string
tuple2str (const rtacl::tuple<ipv4a>& t)
{
    return (boost::format("%s, %s, %d, %d, %d, %d")
            % rtacl::ipv4a2s(t.get<0>())
            % rtacl::ipv4a2s(t.get<1>())
            % static_cast<U32>(t.get<2>())
            % static_cast<U32>(t.get<3>())
            % static_cast<U32>(t.get<4>())
            % static_cast<U32>(t.get<5>())).str();
}

/**
 * @name  tuple2str
 * @brief Converts \e rtacl::tuple<ipv6a> to \e std::string
 *
 * @param[in] t IPv6 6-tuple (sa, da, sp, dp, proto, dscp)
 *
 * @retval \b t as \e std::string
 */
inline std::string
tuple2str (const rtacl::tuple<ipv6a>& t)
{
    return (boost::format("%s, %s, %d, %d, %d, %d")
            % rtacl::ipv6a2s(t.get<0>())
            % rtacl::ipv6a2s(t.get<1>())
            % static_cast<U32>(t.get<2>())
            % static_cast<U32>(t.get<3>())
            % static_cast<U32>(t.get<4>())
            % static_cast<U32>(t.get<5>())).str();
}

/**
 * @name  range2str
 * @brief Converts \e rtacl::range<ipv4a> to \e std::string
 *
 * @param[in] r IPv4 range of 6-tuple (sa, da, sp, dp, proto, dscp)
 *
 * @retval \b t as \e std::string
 */
inline std::string
range2str (const rtacl::range<ipv4a>& r)
{
    return (boost::format("%s-%s, %s-%s, %d-%d, %d-%d, %d-%d, %d-%d")
            % rtacl::ipv4a2s(r.min_corner().get<0>() + 1)
            % rtacl::ipv4a2s(r.max_corner().get<0>() - 1)
            % rtacl::ipv4a2s(r.min_corner().get<1>() + 1)
            % rtacl::ipv4a2s(r.max_corner().get<1>() - 1)
            % static_cast<U32>(r.min_corner().get<2>() + 1)
            % static_cast<U32>(r.max_corner().get<2>() - 1)
            % static_cast<U32>(r.min_corner().get<3>() + 1)
            % static_cast<U32>(r.max_corner().get<3>() - 1)
            % static_cast<U32>(r.min_corner().get<4>() + 1)
            % static_cast<U32>(r.max_corner().get<4>() - 1)
            % static_cast<U32>(r.min_corner().get<5>() + 1)
            % static_cast<U32>(r.max_corner().get<5>() - 1)).str();
}

/**
 * @name  range2str
 * @brief Converts \e rtacl::range<ipv6a> to \e std::string
 *
 * @param[in] r IPv6 range of 6-tuple (sa, da, sp, dp, proto, dscp)
 *
 * @retval \b t as \e std::string
 */
inline std::string
range2str (const rtacl::range<ipv6a>& r)
{
    return (boost::format("%s-%s, %s-%s, %d-%d, %d-%d, %d-%d, %d-%d")
            % ipv6a2s(r.min_corner().get<0>() + 1)
            % ipv6a2s(r.max_corner().get<0>() - 1)
            % ipv6a2s(r.min_corner().get<1>() + 1)
            % ipv6a2s(r.max_corner().get<1>() - 1)
            % static_cast<U32>(r.min_corner().get<2>() + 1)
            % static_cast<U32>(r.max_corner().get<2>() - 1)
            % static_cast<U32>(r.min_corner().get<3>() + 1)
            % static_cast<U32>(r.max_corner().get<3>() - 1)
            % static_cast<U32>(r.min_corner().get<4>() + 1)
            % static_cast<U32>(r.max_corner().get<4>() - 1)
            % static_cast<U32>(r.min_corner().get<5>() + 1)
            % static_cast<U32>(r.max_corner().get<5>() - 1)).str();
}

/*
 * Class member inline functions
 */

/**
 * @name  db<ADDR>::db
 * @brief Constructor
 */
template <class ADDR>
inline
db<ADDR>::db ()
 {
     if (typeid(ADDR) == typeid(rtacl::ipv4a)) {
         af = AF_INET;
         ao = offsetof(sockaddr_in, sin_addr);
         po = offsetof(sockaddr_in, sin_port);
     } else {
         af = AF_INET6;
         ao = offsetof(sockaddr_in, sin_addr);
         po = offsetof(sockaddr_in, sin_port);
     }
 }

/**
 * @name  db<ADDR>::find
 * @brief Public function
 *        Tries to find R-tree entries matching \b key
 *
 * @param ADDR \e rtacl::ipv4a (\e s64) or \e rtacl::ipv6a (\e s256)
 *
 * @param[in] key ACL search key
 *
 * @retval rtacl::result<ADDR> Search result
 */
template <class ADDR>
inline result<ADDR>
db<ADDR>::find (const tuple<ADDR>& key)
{
    result<ADDR> r;
    size_t n = rtree.query(bgi::contains(key), std::back_inserter(r));
    if (n != r.size()) {
        fprintf(stderr, "n(%ld) != r.size()(%ld)\n", n, r.size());
    }
    assert(n == r.size());

    return r;
}

/**
 * @name  db<ADDR>::makeTuple
 * @brief Private function
 *        Makes \e rtacl::tuple<ADDR> from \e sockaddr_in parameters
 *
 * @param ADDR must be \e rtacl::ipv4a (\e s64)
 *
 * @param[in]  src    Source IPv4 address and ports
 * @param[in]  dst    Destination IPv4 address and ports
 * @param[in]  proto  IP Protocol (TCP, UDP, etc.)
 * @param[in]  dscp   The value of DSCP
 * @param[in]  offset One of the followings:
 *                    \b offsetKey, \b offsetMin, or \b offsetMax
 * @param[out] result \e rtacl::tuple<rtacl::ipv4a> containing
 *                    the contents of all input parameters
 */
template <class ADDR>
inline void
db<ADDR>::makeTuple (const sockaddr_in& src,
                     const sockaddr_in& dst,
                     const u8 proto,
                     const u8 dscp,
                     const s32 offset,
                     tuple<rtacl::ipv4a>& result)
{
    assert(af == AF_INET);

    s64 val;

    val = static_cast<s64>(ntohl(src.sin_addr.s_addr)) + offset;
    bg::set<0>(result, val);
    val = static_cast<s64>(ntohl(dst.sin_addr.s_addr)) + offset;
    bg::set<1>(result, val);
    val = static_cast<s64>(ntohs(src.sin_port)) + offset;
    bg::set<2>(result, val);
    val = static_cast<s64>(ntohs(dst.sin_port)) + offset;
    bg::set<3>(result, val);
    val = static_cast<s64>(proto) + offset;
    bg::set<4>(result, val);
    val = static_cast<s64>(dscp) + offset;
    bg::set<5>(result, val);
}

/**
 * @name  db<ADDR>::makeTuple
 * @brief Private function
 *        Makes \e rtacl::tuple<ADDR> from \e sockaddr_in parameters
 *
 * @param ADDR Must be \e rtacl::ipv6a (\e s256)
 *
 * @param[in] src    Source IPv6 address and ports
 * @param[in] dst    Destination IPv6 address and ports
 * @param[in] proto  IP Protocol (TCP, UDP, etc.)
 * @param[in] dscp   The value of DSCP
 * @param[in] offset One of the followings:
 *                   \b offsetKey, \b offsetMin, or \b offsetMax
 *
 * @retval tuple<ADDR> Result
 */
template <class ADDR>
inline void
db<ADDR>::makeTuple (const sockaddr_in6& src,
                     const sockaddr_in6& dst,
                     const u8 proto,
                     const u8 dscp,
                     const s32 offset,
                     tuple<rtacl::ipv6a>& result)
{
    assert(af == AF_INET6);

    s256 val;

    val = sin6a2int<s256>(src) + offset;
    bg::set<0>(result, val);
    val = sin6a2int<s256>(dst) + offset;
    bg::set<1>(result, val);
    val = static_cast<s256>(ntohs(src.sin6_port)) + offset;
    bg::set<2>(result, val);
    val = static_cast<s256>(ntohs(dst.sin6_port)) + offset;
    bg::set<3>(result, val);
    val = static_cast<s256>(proto) + offset;
    bg::set<4>(result, val);
    val = static_cast<s256>(dscp) + offset;
    bg::set<5>(result, val);
}

/**
 * @name  db<ADDR>::makeTuple
 * @brief Private function
 *        Makes \e rtacl::tuple<ADDR> from \b ADDR parameters
 *
 * @param ADDR must be \e rtacl::ipv6a (\e s256)
 *
 * @param[in] sa     Source IP address (depending on \b ADDR)
 * @param[in] da     Destination IP address
 * @param[in] sp     Source port number
 * @param[in] dp     Destination port number
 * @param[in] proto  IP Protocol (TCP, UDP, etc.)
 * @param[in] dscp   The value of DSCP
 * @param[in] offset One of the followings:
 *                   \e offsetKey, \e offsetMin, or \e offsetMax
 *
 * @retval tuple<ADDR> Result
 */
template <class ADDR>
inline void
db<ADDR>::makeTuple (const ADDR sa, const ADDR da,
                     const ADDR sp, const ADDR dp,
                     const ADDR proto, const ADDR dscp,
                     const s32 offset, tuple<ADDR>& result)
{
    bg::set<0>(result, sa + offset);
    bg::set<1>(result, da + offset);
    bg::set<2>(result, sp + offset);
    bg::set<3>(result, dp + offset);
    bg::set<4>(result, proto + offset);
    bg::set<5>(result, dscp + offset);
}

/**
 * @name  db<ADDR>::dump
 * @brief Public function
 *        Returns a copy of the entire R-tree entries
 *
 * @param ADDR \e rtacl::ipv4a (\e s64) or \e rtacl::ipv6a (\e s256)
 *
 * @retval rtacl::result<ADDR> A copy of the entire entries in \b db<ADDR>
 */
template <class ADDR>
inline result<ADDR>
db<ADDR>::dump () const
{
    /*
     * Get an entire copy of the entries
     */
    range<ADDR>  b = rtree.bounds();
    result<ADDR> r;
    rtree.query(bgi::covered_by(b), std::back_inserter(r));

    return r;
}

/**
 * @name  sockItem<ADDR>::sockItem
 * @brief Constructor
 */
template <class SADDR>
sockItem<SADDR>::sockItem ()
{
    memset(&src, 0, sizeof(src));
    memset(&dst, 0, sizeof(dst));
    sockaddr* ps = (sockaddr*)(&src);
    sockaddr* pd = (sockaddr*)(&dst);

    if (typeid(SADDR) == typeid(sockaddr_in)) {
        ps->sa_family = AF_INET;
        pd->sa_family = AF_INET;
        af = AF_INET;
        ao = offsetof(sockaddr_in, sin_addr);
        po = offsetof(sockaddr_in, sin_port);
    } else if (typeid(SADDR) == typeid(sockaddr_in6)) {
        ps->sa_family = AF_INET6;
        pd->sa_family = AF_INET6;
        af = AF_INET6;
        ao = offsetof(sockaddr_in6, sin6_addr);
        po = offsetof(sockaddr_in6, sin6_port);
    } else {
        assert(1);
    }
}

/**
 * @name  sockItem<SADDR>::str
 * @brief Public function
 *        Converts the 6-tuple (sa, da, sp, dp, proto, dscp)
 *        to \e std::string
 *
 * @param SADDR Must be either \e sockaddr_in or \e sockaddr_in6
 *
 * @retval std::string The 6-tuple in \e std::string
 */
template <class SADDR>
inline std::string
sockItem<SADDR>::str ()
{
    char src[64];
    char dst[64];
    inet_ntop(af, getsa(), src, sizeof(src));
    inet_ntop(af, getda(), dst, sizeof(dst));
    return (boost::format("%s, %s, %d, %d, %d, %d")
            % src
            % dst
            % (u32)(getsp())
            % (u32)(getdp())
            % (u32)(getProto())
            % (u32)(getDSCP())).str();

}

/**
 * @name  sockEnt<SADDR>::str
 * @brief Public function
 *        Converts the range of 6-tuple (sa, da, sp, dp, proto, dscp)
 *        to \e std::string
 *
 * @param SADDR Must be either \e sockaddr_in or \e sockaddr_in6
 *
 * @retval std::string The range of 6-tuple in \e std::string
 */
template <class SADDR>
inline std::string
sockEnt<SADDR>::str ()
{
    char saLo[64];
    char saHi[64];
    char daLo[64];
    char daHi[64];

    inet_ntop(min.af, min.getsa(), saLo, sizeof(saLo));
    inet_ntop(min.af, min.getda(), daLo, sizeof(daLo));
    inet_ntop(max.af, max.getsa(), saHi, sizeof(saHi));
    inet_ntop(max.af, max.getda(), daHi, sizeof(daHi));
    return (boost::format("%s-%s, %s-%s, %d-%d, %d-%d, %d-%d, %d-%d")
            % saLo
            % saHi
            % daLo
            % daHi
            % (u32)(min.getsp())
            % (u32)(max.getsp())
            % (u32)(min.getdp())
            % (u32)(max.getdp())
            % (u32)(min.getProto())
            % (u32)(max.getProto())
            % (u32)(min.getDSCP())
            % (u32)(max.getDSCP())).str();
}


} //namespace 
#endif// __RTACL_HPP__
