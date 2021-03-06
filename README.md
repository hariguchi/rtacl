# rtacl::db - An R-tree based ACL


## Introduction

**rtacl::db** is an ACL (Access Control List) C++ class using
R-tree. **rtacl::db** supports both IPv4 and IPv6 address
ranges as well as TCP/UDP port range, IP protocol range, and
DSCP range although IP protocol and DSCP may not need range
matching.


## Requirements

* **rtacl::db** uses **boost::geometry::index::rtree**. It is
  necessary to install [boost C++ libraries](http://www.boost.org/).
  Version 1.63 (or newer) is preferable.
* *local_types.h* must be stored in the include path as well as
  *rtacl.hpp*


## Performance

* CPU: Intel(R) Core(TM) i7-7700HQ CPU @ 2.80GHz
* Memory: 16GB
* OS: Darwin Kernel Version 16.7.0: root:xnu-3789.70.16~2/RELEASE_X86_64 x86_64
* Compiler: Apple LLVM version 9.0.0 (clang-900.0.38)
* Compiler options: -O3 -DNDEBUG


### Search (matching)

730 ns/search with 1 million IPv4 ACL entries.


### Search (un-matching)

610 ns/search with 1 million IPv4 ACL entries.


### Insertion

2.49 us/insertion in average while inserting 1 million IPv4 ACL
entries (ascendant.)


### Deletion

850 ns/insertion in average while deleting 1 million IPv4 ACL
entries (ascendant.)


## Data Structures

* **rtacl::ipv4a**: IPv4 address to be used inside
  **rtacl::db**, which is a *typedef* of **int64_t**. This must
  be in the host byte order.
* **rtacl::ipv6a**: IPv6 address to be used inside
  **rtacl::db**, which is a *typedef* of
  **boost::multiprecision::int256_t**. This must be in the
  host byte order.
* **rtacl::tuple<ADDR>**: ACL tuple (search key.) Template
  parameter **ADDR** must be either **rtacl::ipv4a** or
  **rtacl::ipv6a**. This is a *typedef* of
  **boost::geometry::point** (6 dimensional.) Its order must be:
    1. source IPv4/IPv6 address
    2. destination IPv4/IPv6 address
    3. source port number
    4. destination port number
    5. IP Protocol
    6. DSCP
* **rtacl::range<ADDR>**: The ACL range defined by two
  **rtacl::tuple**s (minimum and maximum). Template parameter
  **ADDR** must be either **rtacl::ipv4a** or **rtacl::ipv6a**.
  This is a *typedef* of **boost::geometry::model::box**, i.e.,
  a multi-dimensional polygon defined by two multi-dimensional
  points.
* **rtacl::entry<ADDR>**: R-tree ACL entry. Template parameter
  **ADDR** must be either **rtacl::ipv4a** or
  **rtacl::ipv6a**. **rtacl::entry** is a *typedef* of
  **std::pair** comprising **rtacl::range** (first type) and
  the associated unique index (second type.) R-tree is an
  algorithm to index multi-dimensional polygons. Index can 
  be any integer as long as it is unique to the associated
  **rtacl::emtry**. However, it is typical for users to prepare
  their own ACL entry data structure associated 
  with **rtacl::entry** (see *unitTest.cpp*) so that index is
  usually the pointer of user-defined ACL entry associated with
  an **rtacl::entry**.
* **rtacl::result<ADDR>**: R-tree ACL search result. Template
  parameter **ADDR** must be either **rtacl::ipv4a** or
  **rtacl::ipv6a**. **rtacl::result<ADDR>** is a *typedef* of
  **std::vector<rtacl::entry<ADDR>>**.

* **rtacl::db<ADDR>**: R-tree ACL class. Template parameter
  **ADDR** must be either **rtacl::ipv4a** or **rtacl::ipv6a**.
  **rtacl::db** is not intrusive.


## rtacl::db<ADDR>


### Template Parameters

* **ADDR**: must be either **rtacl::ipv4a** (**s64**) or
  **rtacl::ipv6a** (**boost::multiprecision::int256_t**.)


### Member Functions

```C++
template <class ADDR>
void rtacl::db::makeMin(const ADDR sa,
                        const ADDR da,
                        const ADDR sp,
                        const ADDR dp,
                        const ADDR proto,
                        const ADDR dscp,
                        tuple<ADDR>& result);
```

Makes the lower bound of an R-tree ACL entry


##### Template Parameters

* **ADDR**: must be either **rtacl::ipv4a** (**s64**) or
  **rtacl::ipv6a** (**boost::multiprecision::int256_t**.)


##### Input Parameters

* **sa**: Source IP address (IPv4 or IPv6 depending on **ADDR**)
* **da**: Destination IP address (IPv4 or IPv6 depending on **ADDR**)
* **sp**: Source port number
* **dp**: Destination port number
* **proto**: IP Protocol number (6 for TCP, 17 for UDP, etc.)
* **dscp**: DSCP


##### Output Parameters

* **result**: Lower bound R-tree ACL tuple representing the
  input parameters for insertion and deletion.


```C++
template <class ADDR>
void rtacl::db::makeMax(const ADDR sa,
                        const ADDR da,
                        const ADDR sp,
                        const ADDR dp,
                        const ADDR proto,
                        const ADDR dscp,
                        tuple<ADDR>& result);
```

Makes the upper bound of an R-tree ACL entry


##### Template Parameters

* **ADDR**: must be either **rtacl::ipv4a** (**s64**) or
  **rtacl::ipv6a** (**boost::multiprecision::int256_t**.)


##### Input Parameters

* **sa**: Source IP address (IPv4 or IPv6 depending on **ADDR**)
* **da**: Destination IP address (IPv4 or IPv6 depending on **ADDR**)
* **sp**: Source port number
* **dp**: Destination port number
* **proto**: IP Protocol number (6 for TCP, 17 for UDP, etc.)
* **dscp**: DSCP

##### Output Parameters

* **result**: Upper bound R-tree ACL tuple representing the
  input parameters for insertion and deletion.


```C++
template <class ADDR>
void rtacl::db::insert(entry<ADDR> const& ent);
```

Inserts a copy of **ent** into **db::rtree**.


##### Template Parameters

* **ADDR**: must be either **rtacl::ipv4a** (**s64**) or
  **rtacl::ipv6a** (**boost::multiprecision::int256_t**.)


##### Input Parameters

* **ent**: Reference to the R-tree ACL entry to be inserted.


```C++
template <class ADDR>
void rtacl::db::remove(entry<ADDR> const& ent);
```

Removes the R-tree entry matching **ent** from **db::rtree**.


##### Template Parameters

* **ADDR**: must be either **rtacl::ipv4a** (**s64**) or
  **rtacl::ipv6a** (**boost::multiprecision::int256_t**.)


##### Input Parameters

* **ent**: Reference to the R-tree ACL entry to be removed.


```C++
template <class ADDR>
rtacl::result<ADDR> rtacl::db::find(const tuple<ADDR>& key);
```

Finds R-tree ACL entries matching **key**.


##### Template Parameters

* **ADDR**: must be either **rtacl::ipv4a** (**s64**) or
  **rtacl::ipv6a** (**boost::multiprecision::int256_t**.)


##### Input Parameters

* **key**: Reference to the R-tree ACL tuple to be looked up.


##### Return Value

Vector of the matched R-tree ACL entries.


```C++
template <class ADDR>
rtacl::result<ADDR> rtacl::db::dump();
```


##### Return Value

Vector of all the R-tree ACL entries.


## Examples

The following function is a part of *unitTest.cpp*.

```C++
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
```


## Performance Details

* CPU: Intel(R) Core(TM) i3-4160T CPU @ 3.10GHz (cache size: 3072 KB)
* Memory: 4GB
* Compiler options: -O3 -DNDEBUG


### Search (matching)

830ns/search with 1 million IPv4 ACL entries.


### Search (un-matching)

780ns/search with 1 million IPv4 ACL entries.


### Insertion

3.27 us/insertion in average while inserting 1 million IPv4 ACL entries.


### Deletion

1.25 us/insertion in average while deleting 1 million IPv4 ACL entries.


```
CPU: Intel(R) Core(TM) i7-7700HQ CPU @ 2.80GHz
Memory: 16GB
OS: Darwin Kernel Version 16.7.0: root:xnu-3789.70.16~2/RELEASE_X86_64 x86_64
Compiler Options: -O3 -DNDEBUG


size: 1000000, i: 1000000
insert:  1000000 calls, 2492031.59 us, 2.49 us/call, min: 70 ns, max: 187.717 us
insert:      0ns -  100ns:  0.00%  14
insert:    100ns -  200ns:  0.00%  38
insert:    200ns -  300ns:  0.01%  64
insert:    300ns -  400ns:  0.02%  172
insert:    400ns -  500ns:  0.03%  319
insert:    500ns -  600ns:  0.10%  973
insert:    600ns -  700ns:  0.19%  1895
insert:    700ns -  800ns:  0.53%  5297
insert:    800ns -  900ns:  1.64%  16423
insert:    900ns - 1000ns:  2.69%  26923
insert:      1us -  2us:   78.90%  788990
insert:      2us -  3us:    3.71%  37134
insert:      3us -  4us:    0.29%  2875
insert:      4us -  5us:    0.00%  46
insert:      5us -  6us:    0.00%  25
insert:      6us -  7us:    0.23%  2282
insert:      7us -  8us:    3.09%  30920
insert:      8us -  9us:    3.52%  35159
insert:      9us - 10us:    1.55%  15544
insert:     10us -  20us:   2.87%  28701
insert:     20us -  30us:   0.46%  4614
insert:     30us -  40us:   0.11%  1056
insert:     40us -  50us:   0.03%  268
insert:     50us -  60us:   0.02%  168
insert:     60us -  70us:   0.01%  63
insert:     70us -  80us:   0.00%  21
insert:     80us -  90us:   0.00%  7
insert:     90us - 100us:   0.00%  3
insert:    100us - 1000us:  0.00%  6
insert:           >1ms:      ---   0

Random match test:
match:  1000000 calls, 729151.01 us, 0.73 us/call, min: 227 ns, max: 79.89 us
match:      0ns -  100ns:  0.00%  0
match:    100ns -  200ns:  0.00%  0
match:    200ns -  300ns:  0.01%  92
match:    300ns -  400ns:  0.13%  1346
match:    400ns -  500ns:  1.81%  18099
match:    500ns -  600ns: 14.47%  144746
match:    600ns -  700ns: 33.31%  333141
match:    700ns -  800ns: 28.82%  288204
match:    800ns -  900ns: 13.54%  135388
match:    900ns - 1000ns:  4.54%  45410
match:      1us -  2us:    3.28%  32753
match:      2us -  3us:    0.00%  6
match:      3us -  4us:    0.00%  5
match:      4us -  5us:    0.00%  14
match:      5us -  6us:    0.00%  21
match:      6us -  7us:    0.00%  39
match:      7us -  8us:    0.00%  9
match:      8us -  9us:    0.00%  13
match:      9us - 10us:    0.00%  14
match:     10us - 100us:   0.07%  700
match:    100us - 1000us:  0.00%  0
match:           >1ms:      ---   0

Random unmatch test:
unmatch:  1000000 calls, 611945.79 us, 0.61 us/call, min: 61 ns, max: 79.631 us
unmatch:      0ns -  100ns:  0.01%  52
unmatch:    100ns -  200ns:  0.16%  1631
unmatch:    200ns -  300ns:  0.78%  7849
unmatch:    300ns -  400ns:  3.46%  34551
unmatch:    400ns -  500ns: 14.00%  139996
unmatch:    500ns -  600ns: 31.79%  317911
unmatch:    600ns -  700ns: 29.74%  297396
unmatch:    700ns -  800ns: 14.14%  141438
unmatch:    800ns -  900ns:  4.19%  41924
unmatch:    900ns - 1000ns:  1.19%  11918
unmatch:      1us - 10us:    0.48%  4833
unmatch:     10us - 100us:   0.05%  501
unmatch:    100us - 1000us:  0.00%  0
unmatch:           >1ms:      ---   0

remove:  999986 calls, 845860.99 us, 0.85 us/call, min: 53 ns, max: 954.782 us
remove:      0ns -  100ns:  0.00%  26
remove:    100ns -  200ns:  0.06%  626
remove:    200ns -  300ns:  3.19%  31913
remove:    300ns -  400ns: 38.99%  389933
remove:    400ns -  500ns: 30.51%  305125
remove:    500ns -  600ns:  7.67%  76657
remove:    600ns -  700ns:  4.23%  42281
remove:    700ns -  800ns:  1.80%  18030
remove:    800ns -  900ns:  1.07%  10669
remove:    900ns - 1000ns:  0.43%  4308
remove:      1us -  2us:    0.39%  3892
remove:      2us -  3us:    4.37%  43692
remove:      3us -  4us:    4.02%  40166
remove:      4us -  5us:    1.27%  12706
remove:      5us -  6us:    1.05%  10512
remove:      6us -  7us:    0.44%  4446
remove:      7us -  8us:    0.20%  1983
remove:      8us -  9us:    0.09%  931
remove:      9us - 10us:    0.05%  545
remove:     10us - 100us:   0.15%  1523
remove:    100us - 1000us:  0.00%  22
remove:           >1ms:      ---   14

```


## Sockaddr Interface

*rtacl.h* has supporting classes using **sockaddr_in** and
**sockaddr_in6** for **rtacl::db** although it is not necessary
to use them.


## rtacl::sockItem<SADDR>

ACL tuple using **sockaddr_in** or **sockaddr_in6**.


### Template Parameters

* **SADDR**: must be either **sockaddr_in** or
  **sockaddr_in6**.


### Member Functions

```C++
template <class SADDR>
void rtacl::sockItem::set (SADDR& src,
                           SADDR& dst,
                           u8 proto,
                           u8 dscp);
```

Sets the followings:
  * source IP address and port (**sockaddr_in** or **sockaddr_in6**)
  * destination IP address and port (**sockaddr_in** or **sockaddr_in6**)
  * IP Protocol number (6 for TCP, 17 for UDP, etc.)
  * DSCP


##### Template Parameters

* **SADDR** must be either **sockaddr_in** or **sockaddr_in6**.


##### Input Parameters

* **src**: Source IP address and port number
* **dst**: Destination IP address andd port number
* **proto**: IP Protocol number (6 for TCP, 17 for UDP, etc.)
* **dscp**: DSCP


```C++
template <class SADDR>
void rtacl::sockItem::setSrc (SADDR& src);
```

Sets source IP address and port number.


##### Template Parameters

* **SADDR** must be either **sockaddr_in** or **sockaddr_in6**.


##### Input Parameters

* **src**: Source IP address and port number


```C++
template <class SADDR>
void rtacl::sockItem::setDst (SADDR& dst);
```

Sets destination IP address and port number.


##### Template Parameters

* **SADDR** must be either **sockaddr_in** or **sockaddr_in6**.


##### Input Parameters

* **dst**: Destination IP address and port number


```C++
template <class SADDR>
void rtacl::sockItem::setProto (u8 proto);
```

Sets IP protocol number.


##### Template Parameters

* **SADDR** must be either **sockaddr_in** or **sockaddr_in6**.


##### Input Parameters

* **proto**: IP Protocol number (6 for TCP, 17 for UDP, etc.)


```C++
template <class SADDR>
void rtacl::sockItem::setDSCP (u8 dscp);
```

Sets DSCP value.


##### Template Parameters

* **SADDR** must be either **sockaddr_in** or **sockaddr_in6**.


##### Input Parameters

* **dscp**: DSCP


```C++
template <class SADDR>
SADDR& rtacl::sockItem::getSrc ();
```

Returns the reference to the source **SADDR**


##### Template Parameters

* **SADDR** must be either **sockaddr_in** or **sockaddr_in6**.


##### Return Value

* **SADDR&**: Reference to the source **SADDR**


```C++
template <class SADDR>
SADDR& rtacl::sockItem::getDst ();
```

Returns the reference to the destination **SADDR**


##### Template Parameters

* **SADDR** must be either **sockaddr_in** or **sockaddr_in6**.


##### Return Value

* **SADDR&**: Reference to the destination **SADDR**


```C++
template <class SADDR>
void* rtacl::sockItem::getsa ();
```

Returns the pointer to the source **sin_addr** or **sin6_addr**
depending on **SADDR**.


##### Template Parameters

* **SADDR** must be either **sockaddr_in** or **sockaddr_in6**.


##### Return Value

* **void***: Pointer to the source **sin_addr** or **sin6_addr**


```C++
template <class SADDR>
void* rtacl::sockItem::getda ();
```

Returns the pointer to the destination **sin_addr** or **sin6_addr**
depending on **SADDR**.


##### Template Parameters

* **SADDR** must be either **sockaddr_in** or **sockaddr_in6**.


##### Return Value

* **void***: Pointer to the destination **sin_addr** or **sin6_addr**


```C++
template <class SADDR>
u16 rtacl::sockItem::getsp ();
```

Returns the source port number in the host byte order.


##### Template Parameters

* **SADDR** must be either **sockaddr_in** or **sockaddr_in6**.


##### Return Value

* **u16**: Source port number in the host byte order


```C++
template <class SADDR>
u16 rtacl::sockItem::getdp ();
```

Returns the destination port number in the host byte order.


##### Template Parameters

* **SADDR** must be either **sockaddr_in** or **sockaddr_in6**.


##### Return Value

* **u16**: Destination port number in the host byte order


```C++
template <class SADDR>
u8 rtacl::sockItem::getProto ();
```

Returns the IP Protocol number.


##### Template Parameters

* **SADDR** must be either **sockaddr_in** or **sockaddr_in6**.


##### Return Value

* **u8**: IP Protocol number


```C++
template <class SADDR>
u8 rtacl::sockItem::getDSCP ();
```

Returns the DSCP value.


##### Template Parameters

* **SADDR** must be either **sockaddr_in** or **sockaddr_in6**.


##### Return Value

* **u8**: DSCP value


```C++
template <class SADDR>
std::string rtacl::sockItem::str ();
```

Converts the 6-tuple into a string.


##### Template Parameters

* **SADDR** must be either **sockaddr_in** or **sockaddr_in6**.


##### Return Value

* **std::string**: 6-tuple in the following format: sa, da, sp, dp, ipproto, dscp


## rtacl::sockEnt<SADDR>

ACL entry using **sockaddr_in** or **sockaddr_in6**.


### Template Parameters

* **SADDR**: must be either **sockaddr_in** or
  **sockaddr_in6**.


### Member Functions

```C++
template <class SADDR>
rtal::sockEnt::set (sockItem<SADDR>& min, sockItem<SADDR>& max, u32 pri);
```

Sets the ACL entry from the input parameters.


##### Template Parameters

* **SADDR** must be either **sockaddr_in** or **sockaddr_in6**.


##### Input Parameters

* **min**: Lower bound of the ACL entry
* **max**: Upper bound of the ACL entry


```C++
template <class SADDR>
rtal::sockEnt::setMin (sockItem<SADDR>& min);
```

Sets the lower bound of the ACL entry.


##### Template Parameters

* **SADDR** must be either **sockaddr_in** or **sockaddr_in6**.


##### Input Parameters

* **min**: Lower bound of the ACL entry


```C++
template <class SADDR>
rtal::sockEnt::setMax (sockItem<SADDR>& max);
```

Sets the upper bound of the ACL entry.


##### Template Parameters

* **SADDR** must be either **sockaddr_in** or **sockaddr_in6**.


##### Input Parameters

* **max**: Upper bound of the ACL entry


```C++
template <class SADDR>
rtal::sockEnt::setPriority (u32 pri);
```

Sets ACL entry's priority as the tie breaker when multiple ACL
entries are hit.


##### Template Parameters

* **SADDR** must be either **sockaddr_in** or **sockaddr_in6**.


##### Input Parameters

* **pri**: Priority (users can choose either larger number has
  higher priority or lower number has higher priority.))


```C++
template <class SADDR>
std::string rtacl::sockEnt::str ();
```

Converts an ACL entry into a string.


##### Template Parameters

* **SADDR** must be either **sockaddr_in** or **sockaddr_in6**.


##### Return Value

* **std::string**: SAmin-SAmax, DAmin-DAmax, SPmin-SPmax, DPmin-DPmax, PROTOmin-PROTOmax, DSCPmin-DSCPmax


## Supporting Functions


```C++
inline std::string
ipv4a2s (const rtacl::ipv4a a);
```

Converts IPv4 address (in **rtacl::ipv4a**) to a string.


### Input Parameters

* **a**: IPv4 address


### Return Value

* **std::string**: IPv4 address string (e.g., "1.2.3.4")


```C++
inline std::string
ipv6a2s (const rtacl::ipv6a& a);
```

Converts IPv6 address (in **rtacl::ipv6a**) to a string.


### Input Parameters

* **a**: Reference to an IPv6 address (in **rtacl::ipv6a**)


### Return Value

* **std::string**: IPv6 address string (e.g., "2001:0:0:1::1")


```C++
template <class INT>
inline INT
sin6a2int (const sockaddr_in6& sin6);
```

Converts IPv6 address (in **sockaddr_in6**) to either **ipv6a**
(**u128**) or **rtacl::ipv6a** (**s256) (host byte order)


### Template Parameters

* **INT**: Must be either **ipv6a** (**u128**) or **rtacl::ipv6a** (**s256)


### Input Parameters

* **sin6**: Reference to an IPv6 address (in **sockaddr_in6**)


### Return Value

* **INT**: IPv6 address in the host byte order


```C++
template <class INT>
inline sockaddr_in6
int2sin6 (const INT& addr);
```

Converts IPv6 address (in either **ipv6a** (**u128**) or
**rtacl::ipv6a** (**s256) (host byte order)) to **sockaddr_in6**.


### Template Parameters

* **INT**: Must be either **ipv6a** (**u128**) or **rtacl::ipv6a** (**s256**)


### Input Parameters

* **addr**: Reference to an IPv6 address in **INT**


### Return Value

* **sockaddr_in6**: IPv6 address in the network byte order


```C++
inline std::string
tuple2str (const rtacl::tuple<ipv4a>& t);
```

Converts IPv4 6-tuple to a string.


### Input Parameters

* **t**: Reference to an Ipv4 6-tuple


### Return Value

* **std::string**: Source Ipv4 address, destination Ipv4
                   address, source port number, destination
                   port number, IP Protocol number, DSCP
                   (e.g., "1.2.3.4, 5.6.7.8, 12345, 80, 6, 0")


```C++
inline std::string
tuple2str (const rtacl::tuple<ipv6a>& t);
```

Converts IPv6 6-tuple to a string.


### Input Parameters

* **t**: Reference to an IPv6 6-tuple


### Return Value

* **std::string**: Source IPv6 address, destination IPv6
                   address, source port number, destination
                   port number, IP Protocol number, DSCP
                   (e.g., "2001:0:0:1::1, 2001:0:0:2::1, 12345, 80, 6, 0")


```C++
inline std::string
range2str (const rtacl::range<ipv4a>& r);
```

Converts IPv4 6-tuple range to a string.


### Input Parameters

* **r**: Reference to an Ipv4 6-tuple range


### Return Value

* **std::string**: Source Ipv4 address range, destination Ipv4
                   range, source port range, destination
                   port range, IP Protocol range, DSCP range
                   (e.g., "1.2.3.4-1.2.3.10, 5.6.7.8-5.6.7.15, 12345-23456, 80-80, 6-6, 0-255")


```C++
inline std::string
range2str (const rtacl::range<ipv6a>& r);
```

Converts IPv6 6 tuple range to a string.


### Input Parameters

* **r**: Reference to a range Ipv6 6 range


### Return Value

* **std::string**: Source Ipv6 address range, destination Ipv6
                   range, source port range, destination
                   port range, IP Protocol range, DSCP range
                   (e.g., "2001:0:0:1::1-2001:0:0:1::ffff, 2001:0:0:2::1-2001:0:0:2::ffff, 12345-23456, 80-80, 6-6, 0-255")


## References

* [Original R-tree paper]
   (http://www-db.deis.unibo.it/courses/SI-LS/papers/Gut84.pdf)
* [Overview of R-tree]
   (http://dblab.usc.edu/csci585/585%20materials/RTrees.ppt)
